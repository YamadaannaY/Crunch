//兵营，主要功能 1：具有一个阵营ID，将其赋予其SpawnSpot生成的所有Minion 2：维护一个Pool，从中取出Minion

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "MinionBarrack.generated.h"

UCLASS()
class AMinionBarrack : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditAnywhere,Category="Spawn")
	FGenericTeamId BarrackTeamId;

	UPROPERTY(EditAnywhere,Category="Spawn")
	float GroupSpawnInterval=5.f;

	UPROPERTY(EditDefaultsOnly,Category="Spawn")
	int MinionPerGroup=3;

	UPROPERTY(EditAnywhere,Category="Spawn")
	AActor* Goal;
	
	//Pool机制：由于AI需要进行大量的销毁和生成，对每一个AI单位都进行这样的操作十分消耗性能，造成卡顿和内存抖动，所以使用一个Pool存储生成
	//的所有AI类，只在开始时生成一批，后续的Dead和Spawn都是基于这些存储好的Minion实例来操作，减小CPU负担。
	UPROPERTY()
	TArray<class AMinion*> MinionPool;

	UPROPERTY(EditAnywhere,Category="Spawn")
	TSubclassOf<AMinion> MinionClass;

	UPROPERTY(EditAnywhere,Category="Spawn")
	TArray<APlayerStart*> SpawnSpots;

	UPROPERTY(EditDefaultsOnly,Category="Spawn")
	int MaxSpawnNums = 15;

	int NextSpawnSpotIndex=-1;

	FTimerHandle SpawnIntervalTimerHandle;
	
	//记录在Barrack中的PlayerStart，根据Index获得位置
	const APlayerStart* GetNextSpawnSpot();

	//从Pool中找到可以处于StatDead状态下的Minion
	AMinion* GetNextAvailableMinion() const ;

	//根据PerGroup生成一个新Group,优先从Pool中获取Minion对象
	void SpawnNewGroup();

	//生成Amt个Minion对象
	void SpawnNewMinion(int Amt);
};
