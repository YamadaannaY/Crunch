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
		GetWorld()->ServerTravel(LevelURL.ToString()+"?listen");
	}
}
