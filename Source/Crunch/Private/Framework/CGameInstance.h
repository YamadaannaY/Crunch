#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	//点击Button调用此函数从而加载GameMap
	void StartMatch();
	
private:
	UPROPERTY(EditDefaultsOnly,Category="Map")
	TSoftObjectPtr<UWorld> MainMenuLevel;
	
	UPROPERTY(EditDefaultsOnly,Category="Map")
	TSoftObjectPtr<UWorld> LobbyLevel;
	
	UPROPERTY(EditDefaultsOnly,Category="Map")
	TSoftObjectPtr<UWorld> GameLevel;
	
	//加载GameMap
	void LoadLevelAndListen(TSoftObjectPtr<UWorld> Level);
};
