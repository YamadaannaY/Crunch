#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/GameModeBase.h"
#include "CGameMode.generated.h"

struct FGenericTeamId;
/**
 * 
 */
UCLASS()
class ACGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	//在服务器端生成一个PlayerController实例,客户端会收到这个Controller的镜像（也是一个实例）
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

	//GameMode的BeginPlay会在World开始Play时调用，比所有Actor早,适合确定需要在游戏开始就确定的逻辑
	virtual void StartPlay() override;
	
	//在服务端上为每个Controller调用一次,确定生成的Pawn，在SpawnPlayerController之后调用
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	
private:
	//在OnProperty函数中保留了Lobby中继承的PlayerSelection，如果有则根据其Slot变量获取TeamID,如果没有静态递增赋值
	FGenericTeamId GetTeamIDForPlayer(const AController* InController) const;

	//根据当前ControllerID的Map对应Tag遍历所有StartPoint，找到没有被使用的点并返回，作为Controller的生成点
	AActor* FindNextStartSpotTeam(const FGenericTeamId TeamID) const;

	//让TeamID和设置的StartPoint中的TagName对应映射
	UPROPERTY(EditDefaultsOnly,Category="Team")
	TMap<FGenericTeamId,FName> TeamStartSpotTagMap;
	
	//默认Pawn，在某些没有找到玩家选择的Hero的情况下选择的Hero，如直接在GameLevel中启动
	UPROPERTY(EditDefaultsOnly , Category="Team")
	TSubclassOf<APawn> BackupPawn1;
	
	UPROPERTY(EditDefaultsOnly,Category="Team")
	TSubclassOf<APawn> BackupPawn2;

	//ActorIterator迭代器找到World中第一个StormCore
	class AStormCore* GetStormCore() const ;

	//对服务端的所有PlayerController调用MatchFinished，传参数ViewTarget和WinningTeam
	void MatchFinished(AActor* ViewTarget,int WinningTeam) const ;
};
