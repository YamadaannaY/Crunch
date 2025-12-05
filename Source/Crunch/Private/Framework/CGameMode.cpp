// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CGameMode.h"
#include "EngineUtils.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerStart.h"

APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	//获得生成的Controller
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);

	//获取Controller继承的TeamInterface处理TeamID
	IGenericTeamAgentInterface* NewPlayerTeamInterface=Cast<IGenericTeamAgentInterface>(NewPlayerController);
	//为Controller分配ID
	const FGenericTeamId TeamId=GetTeamIDForPlayer(NewPlayerController);
	
	//为当前生成的Controller分配一个ID
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
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag=FName("Taken");
			return *It;
		}
	}

	return nullptr;
}
