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
		//没有DeadTag视为激活
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
		
		//找到DeadTag下的Minion
		AMinion* NextAvailableMinion=GetNextAvailableMinion();
		if (!NextAvailableMinion) break;

		//ReSpawn
		NextAvailableMinion->SetActorTransform(SpawnTransform);
		NextAvailableMinion->Activate();
		--i;
	}

	if (i != 0 && MinionPool.Num()<MaxSpawnNums)
	{
		//意味着当前所有没有DeadTag下的，这种情况下新生成Minion)
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