// Fill out your copyright notice in the Description page of Project Settings.

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
	// Called when the game starts or when spawned
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
	
	//池化机制：由于AI需要进行大量的销毁和生成，对每一个AI单位都进行这样的操作十分消耗性能，造成卡顿和内存抖动，所以使用一个Pool存储生成
	//的所有AI类，只在开始时生成一批，后续的销毁和生成都是基于这些存储好的AI实例来操作，显著减小了CPU负担。
	UPROPERTY()
	TArray<class AMinion*> MinionPool;

	//传入具体的Minion类
	UPROPERTY(EditAnywhere,Category="Spawn")
	TSubclassOf<AMinion> MinionClass;

	UPROPERTY(EditAnywhere,Category="Spawn")
	TArray<APlayerStart*> SpawnSpots;

	int NextSpawnSpotIndex=-1;

	//记录在Barrack中的PlayerStart，根据Index获得位置
	const APlayerStart* GetNextSpawnSpot();

	//从Pool中找到可以处于StatDead状态下的Minion
	AMinion* GetNextAvailableMinion() const ;

	//根据PerGroup生成一个新Group,优先从Pool中获取Minion对象
	void SpawnNewGroup();

	//生成Amt个Minion对象
	void SpawnNewMinion(int Amt);

	//多少秒SpawnNewGroup
	FTimerHandle SpawnIntervalTimerHandle;
};
