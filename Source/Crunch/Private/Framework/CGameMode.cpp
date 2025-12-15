//GameMode是生成Controller的关键类
//适合处理Controller在生成时应该具有的相关信息 eg：TeamID，生成位置 etc..


#include "Framework/CGameMode.h"
#include "EngineUtils.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerStart.h"

APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	//获得生成的Controller
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);

	//获取Controller的TeamInterface接口处理TeamID
	IGenericTeamAgentInterface* NewPlayerTeamInterface=Cast<IGenericTeamAgentInterface>(NewPlayerController);

	//为Controller分配ID
	const FGenericTeamId TeamId=GetTeamIDForPlayer(NewPlayerController);
	
	//将ID赋予Controller
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}
	
	//为当前生成的Controller指定生成点
	NewPlayerController->StartSpot = FindNextStartSpotTeam(TeamId);
	
	return NewPlayerController;
}

FGenericTeamId ACGameMode::GetTeamIDForPlayer(const APlayerController* PlayerController) const
{
	//轮流分配
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
		//ActorType* operator->() return **this
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag=FName("Taken");
			//*this
			return *It;
		}
	}
	return nullptr;
}
