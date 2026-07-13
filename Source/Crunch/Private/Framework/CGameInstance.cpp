#include "CGameInstance.h"
#include "CAssetManager.h"
#include "Character/PA_CharacterDefination.h"

void UCGameInstance::Shutdown()
{
	// PIE 退出 / 游戏关闭时，恢复所有 CDO Mesh 为默认值
	TArray<UPA_CharacterDefination*> AllCharDefs;
	if (UCAssetManager::Get().GetLoadedCharacterDefinition(AllCharDefs))
	{
		for (UPA_CharacterDefination* CharDef : AllCharDefs)
		{
			CharDef->RestoreDefaultMesh();
		}
	}

	Super::Shutdown();
}

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
