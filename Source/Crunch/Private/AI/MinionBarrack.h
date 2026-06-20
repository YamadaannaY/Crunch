#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "MinionBarrack.generated.h"

//兵营，主要功能 1：具有一个阵营ID，将其赋予其SpawnSpot生成的所有Minion 2：维护一个MinionPool，从中取出Minion进行生成
UCLASS(MinimalAPI)
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

	UPROPERTY()
	TArray<class AMinion*> MinionPool;

	UPROPERTY(EditAnywhere,Category="Spawn")
	TSubclassOf<AMinion> MinionClass;

	UPROPERTY(EditAnywhere,Category="Spawn")
	TArray<APlayerStart*> SpawnSpots;

	//兵营生成数量限制
	UPROPERTY(EditAnywhere,Category="Spawn")
	int MaxSpawnNums = 15;

	//StartSpotIndex
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
