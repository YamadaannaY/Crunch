// 跨地图存在的游戏进程唯一的类，在游戏启动时创建，结束时销毁，在这里调用关卡切换逻辑最合适，不会因为关卡切换受到任何影响，其他如Mode,State,World都会进行一次重建

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
