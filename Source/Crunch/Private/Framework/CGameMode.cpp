#include "Framework/CGameMode.h"
#include "EngineUtils.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerStart.h"
#include "Framework/StormCore.h"
#include "Player/CPlayerController.h"
#include "Player/CPlayerState.h"
#include "Character/PA_CharacterDefination.h"
#include "Character/PA_SkinDefination.h"

APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	//获得生成的Controller
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
		
	IGenericTeamAgentInterface* NewPlayerTeamInterface=Cast<IGenericTeamAgentInterface>(NewPlayerController);
		
	//为Controller分配ID
	const FGenericTeamId TeamId=GetTeamIDForPlayer(NewPlayerController);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}
		
	//基于ID选择一个阵营的生成点
	NewPlayerController->StartSpot = FindNextStartSpotTeam(TeamId);
	
	return NewPlayerController;
}

void ACGameMode::StartPlay()
{
	Super::StartPlay();

	AStormCore* StormCore=GetStormCore();
	if (StormCore)
	{
		//绑定回调，决定游戏结束时的逻辑
		StormCore->OnGoalReachedDelegate.AddUObject(this,&ThisClass::MatchFinished);
	}
}

UClass* ACGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	ACPlayerState* CPlayerState = InController->GetPlayerState<ACPlayerState>();

	if (CPlayerState && CPlayerState->GetSelectedPawnClass())
	{
		// 皮肤：SpawnActor 前把 CDO 的 Mesh 改成选中皮肤的 Mesh
		const FPlayerSelection& Selection = CPlayerState->GetPlayerSelection();
		const UPA_CharacterDefination* CharDef = Selection.GetCharacterDefinition();
		if (CharDef)
		{
			CharDef->ApplySkinToClassDefault(Selection.GetSkinDefinition());
		}
		return CPlayerState->GetSelectedPawnClass();
	}

	const IGenericTeamAgentInterface* TeamAgent  = Cast<IGenericTeamAgentInterface>(InController);
	if ( TeamAgent && TeamAgent->GetGenericTeamId()==FGenericTeamId(1))
	{
		return BackupPawn2;
	}

	return BackupPawn1;
}

FGenericTeamId ACGameMode::GetTeamIDForPlayer(const AController* InController) const
{
ACPlayerState* CPlayerState = InController->GetPlayerState<ACPlayerState>();

if (CPlayerState && CPlayerState->GetSelectedPawnClass())
{
	return CPlayerState->GetTeamIdBaseOnSlot();
}
		
//轮流分配，在测试环境下使用
static int PlayerCount=0;
++PlayerCount;
return FGenericTeamId(PlayerCount%2);
}

AActor* ACGameMode::FindNextStartSpotTeam(const FGenericTeamId TeamID) const
{
	//通过当前ControllerID获得对应的StratPoint的Tag
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamID);
	if (!StartSpotTag)
	{
		return nullptr;
	}

	//遍历当前World的PlayerStart，如果当前遍历的点的Tag与通过ID获得的Tag相同，标记为Taken，返回这个点
	const UWorld* World =GetWorld();

	for (TActorIterator<APlayerStart> It(World);It;++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag=FName("Taken"); 
			return *It;
		}
	}
	return nullptr;
}

AStormCore* ACGameMode::GetStormCore() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<AStormCore> It(World);It;)
		{
			return *It;
		}
	}
	return nullptr;
}

void ACGameMode::MatchFinished(AActor* ViewTarget,int WinningTeam) const 
	{
		UWorld* World = GetWorld();
		if (World)
		{
			//遍历服务端所有Controller
			for (TActorIterator<ACPlayerController> It(World);It;++It)
			{
				It->MatchFinished(ViewTarget,WinningTeam);
			}
		}
	}