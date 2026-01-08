// Fill out your copyright notice in the Description page of Project Settings.

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
	//在“服务器端”生成一个PlayerController实例,客户端会收到这个Controller的镜像（也是一个实例）
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

	//属于GameMode的BeginPlay,会在World开始Play时调用，比所有Actor早
	virtual void StartPlay() override;
private:
	//为生成的Controller分配ID，采用一个static值在0,1两个TeamID之间轮流分配（暂时，方便测试）
	FGenericTeamId GetTeamIDForPlayer(const APlayerController*  PlayerController) const;

	//根据当前ControllerID的Map对应Tag遍历所有StartPoint，找到没有被使用的点并返回，作为Controller的生成点
	AActor* FindNextStartSpotTeam(const FGenericTeamId TeamID) const;

	//让TeamID和设置的StartPoint中的TagName对应映射
	UPROPERTY(EditDefaultsOnly,Category="Team")
	TMap<FGenericTeamId,FName> TeamStartSpotTagMap;

	//ActorIterator迭代器找到World中第一个StormCore
	class AStormCore* GetStormCore() const ;

	//对服务端的所有PlayerController调用MatchFinished，传参数ViewTarget和WinningTeam
	UFUNCTION()
	void MatchFinished(AActor* ViewTarget,int WinningTeam);
};
