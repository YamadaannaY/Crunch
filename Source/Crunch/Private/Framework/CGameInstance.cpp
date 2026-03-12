#include "CGameInstance.h"

void UCGameInstance::StartMatch()
{
	if (GetWorld()->GetNetMode() == NM_ListenServer || GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		LoadLevelAndListen(GameLevel);
	}
}

void UCGameInstance::LoadLevelAndListen(TSoftObjectPtr<UWorld> Level)
{
	const FName LevelURL = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));
	
	if (LevelURL != "")
	{
		//当前游戏实例作为ListenServer
		GetWorld()->ServerTravel(LevelURL.ToString()+"?listen");
	}
}
