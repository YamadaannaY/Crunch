#include "GAS/TargetActor_GrounPick.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Crunch/Crunch.h"
#include "Components/DecalComponent.h"
#include "Engine/OverlapResult.h"

ATargetActor_GroundPick::ATargetActor_GroundPick()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Comp"));
	
	DecalComp=CreateDefaultSubobject<UDecalComponent>("Decal Comp");

	//贴花特效
	DecalComp->SetupAttachment(GetRootComponent());
}

void ATargetActor_GroundPick::SetTargetAreaRadius(float NewRadius)
{
	//配置区域半径并使贴花贴合检测区域半径
	TargetAreaRadius=NewRadius;
	
	DecalComp->DecalSize=FVector(NewRadius);	
}

void ATargetActor_GroundPick::ConfirmTargetingAndContinue()
{
	// TA只提供瞄准位置，不做目标选择。目标选择由GA在服务端权威执行。
	FGameplayAbilityTargetDataHandle TargetData;

	FGameplayAbilityTargetData_SingleTargetHit* HitLocation=new FGameplayAbilityTargetData_SingleTargetHit;
	HitLocation->HitResult.ImpactPoint=GetActorLocation();
	TargetData.Add(HitLocation);

	TargetDataReadyDelegate.Broadcast(TargetData);
}

TArray<AActor*> ATargetActor_GroundPick::GetValidTargetsAtLocation(
	UWorld* World,
	FVector Location,
	float Radius,
	const IGenericTeamAgentInterface* OwnerTeamAgent,
	bool bTargetFriendly,
	bool bTargetEnemy)
{
	TArray<AActor*> ValidTargets;
	if (!World) return ValidTargets;

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(Radius);

	World->OverlapMultiByObjectType(OverlapResults, Location, FQuat::Identity, CollisionObjectQueryParams, CollisionShape);

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		if (OwnerTeamAgent)
		{
			const ETeamAttitude::Type Attitude = OwnerTeamAgent->GetTeamAttitudeTowards(*OverlapResult.GetActor());
			if (Attitude == ETeamAttitude::Friendly && !bTargetFriendly) continue;
			if (Attitude == ETeamAttitude::Hostile && !bTargetEnemy) continue;
		}
		ValidTargets.Add(OverlapResult.GetActor());
	}

	return ValidTargets;
}

void ATargetActor_GroundPick::SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy)
{
	bShouldTargetEnemy=bTargetEnemy;
	bShouldTargetFriendly=bTargetFriendly;
}

void ATargetActor_GroundPick::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//主要依赖本地PC进行ViewPort获取位置，所以先检查
	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
	{
		//设置Actor位置
		SetActorLocation(GetTargetPoint());
	}
}

FVector ATargetActor_GroundPick::GetTargetPoint() const
{
	if (!PrimaryPC || !PrimaryPC->IsLocalPlayerController()) return GetActorLocation();

	FHitResult TraceResult;
	FVector ViewLoc;
	FRotator ViewRot;

	//获得摄像机视角原点和朝向
	PrimaryPC->GetPlayerViewPoint(ViewLoc,ViewRot);

	//沿着视口中心延伸TargetTraceRange
	FVector TraceEnd=ViewLoc+ViewRot.Vector()*TargetTraceRange;

	//以Target的碰撞视角进行追踪，找到则返回（墙面）
	GetWorld()->LineTraceSingleByChannel(TraceResult,ViewLoc,TraceEnd,ECC_Target);

	//范围内正前方没有目标（平原），向下找（地面）
	if (!TraceResult.bBlockingHit)
	{
		GetWorld()->LineTraceSingleByChannel(TraceResult,TraceEnd,TraceEnd+FVector::DownVector*TNumericLimits<float>::Max(),ECC_Target);
	}
	//没有碰撞对象，此时判定点不动
	if (!TraceResult.bBlockingHit)
	{
		return GetActorLocation();
	}

	//Debug
	if (bShouldDrawDebug)
	{
		DrawDebugSphere(GetWorld(),TraceResult.ImpactPoint,TargetAreaRadius,32,FColor::Red);
	}

	return TraceResult.ImpactPoint;
}