#include "AI/MinionBarrack.h"
#include "Minion.h"
#include "GameFramework/PlayerStart.h"

void AMinionBarrack::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer
		(SpawnIntervalTimerHandle,this,&ThisClass::SpawnNewGroup,GroupSpawnInterval,true);
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
		if (!Minion->IsActive())
		{
			return Minion;
		}
	}
	return nullptr;
}

void AMinionBarrack::SpawnNewGroup()
{
	int i =MinionPerGroup;
	while (i>0)
	{
		FTransform SpawnTransform = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot=GetNextSpawnSpot())
		{
			SpawnTransform=NextSpawnSpot->GetActorTransform();
		}
		//找到Pool中DeadTag下的Minion进行ReSpawn
		AMinion* NextAvailableMinion=GetNextAvailableMinion();
		if (!NextAvailableMinion) break;
		NextAvailableMinion->SetActorTransform(SpawnTransform);
		NextAvailableMinion->Activate();
		--i;
	}
	if (i != 0 && MinionPool.Num()<MaxSpawnNums)
	{
		//意味着当前没有足够数量DeadTag下的Minion存在，这种情况下新生成Minion，但是不超出Max限制)
		SpawnNewMinion(i);
	}
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

		//延迟生成Actor，因为Actor生成依赖ID来选择Skin，所以要延迟生成，先分配好ID再FinishSpawn
		AMinion* NewMinion=GetWorld()->SpawnActorDeferred<AMinion>(MinionClass,SpawnTransform,this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		NewMinion->SetGenericTeamId(BarrackTeamId);
		NewMinion->FinishSpawning(SpawnTransform);

		//AI的目的地
		NewMinion->SetGoal(Goal);

		//新的Minion要被加到池中
		MinionPool.Add(NewMinion);
	}
}