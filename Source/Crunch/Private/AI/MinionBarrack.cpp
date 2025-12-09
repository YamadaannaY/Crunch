// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MinionBarrack.h"
#include "Minion.h"
#include "GameFramework/PlayerStart.h"

// Called when the game starts or when spawned
void AMinionBarrack::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(SpawnIntervalTimerHandle,this,&ThisClass::SpawnNewGroup,GroupSpawnInterval,true);
	}
}

const APlayerStart* AMinionBarrack::GetNextSpawnSpot()
{
	if (SpawnSpots.Num() == 0) return nullptr;

	++NextSpawnSpotIndex;

	if (NextSpawnSpotIndex>=SpawnSpots.Num()) NextSpawnSpotIndex=0;

	return SpawnSpots[NextSpawnSpotIndex];
}

AMinion* AMinionBarrack::GetNextAvailableMinion() const 
{
	for (AMinion* Minion : MinionPool)
	{
		//InActiveStat
		if (!Minion->IsActive())
		{
			return Minion;
		}
	}
	return nullptr;
}

void AMinionBarrack::SpawnNewGroup()
{
	//Group一次生成多少Minion
	int i =MinionPerGroup;

	while (i>0)
	{
		FTransform SpawnTransform = GetActorTransform();
		
		if (const APlayerStart* NextSpawnSpot=GetNextSpawnSpot())
		{
			SpawnTransform=NextSpawnSpot->GetActorTransform();
		}
		//找到死亡状态的Minion
		AMinion* NextAvailableMinion=GetNextAvailableMinion();
		if (!NextAvailableMinion) break;

		//ReSpawn
		NextAvailableMinion->SetActorTransform(SpawnTransform);
		NextAvailableMinion->Activate();
		--i;
	}

	//意味着当前所有Minion都没有死亡，这种情况下新生成Minion To do：设置上限？）
	SpawnNewMinion(i);
}

void AMinionBarrack::SpawnNewMinion(int Amt)
{
	for (int i=0;i<Amt;++i)
	{
		FTransform SpawnTransform = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot=GetNextSpawnSpot())
		{
			SpawnTransform=NextSpawnSpot->GetActorTransform();
		}

		//生成Actor，但是不会自动完成构造，使调用时能够有机会设置参数
		AMinion* NewMinion=GetWorld()->SpawnActorDeferred<AMinion>(MinionClass,SpawnTransform,this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		NewMinion->SetGenericTeamId(BarrackTeamId);
		//完成构造
		NewMinion->FinishSpawning(SpawnTransform);
		MinionPool.Add(NewMinion);
	}
}