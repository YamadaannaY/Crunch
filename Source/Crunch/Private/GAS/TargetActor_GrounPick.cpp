// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/TargetActor_GrounPick.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"
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
	//以球体检测区域进行Overlap查询
	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(TargetAreaRadius);
	GetWorld()->OverlapMultiByObjectType(OverlapResults,GetActorLocation(),FQuat::Identity,CollisionObjectQueryParams,CollisionShape);

	TSet<AActor*> TargetActors;
	const IGenericTeamAgentInterface* OwnerTeamAgentInterface=nullptr;

	if (OwningAbility)  OwnerTeamAgentInterface=Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		if (OwnerTeamAgentInterface && OwnerTeamAgentInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor())==ETeamAttitude::Friendly && !bShouldTargetFriendly)
		{
			continue;
		}
		if (OwnerTeamAgentInterface && OwnerTeamAgentInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor())==ETeamAttitude::Hostile && !bShouldTargetEnemy)
		{
			continue;
		}

		TargetActors.Add(OverlapResult.GetActor());
	}

	//存储TargetActor到Data中，此Handle会传递给GA中的Confirm回调
	FGameplayAbilityTargetDataHandle TargetData=UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors.Array(),false);

	//获取区域中心的坐标，用于产生Cue
	FGameplayAbilityTargetData_SingleTargetHit* HitLocation=new FGameplayAbilityTargetData_SingleTargetHit;
	HitLocation->HitResult.ImpactPoint=GetActorLocation();
	TargetData.Add(HitLocation);
	
	TargetDataReadyDelegate.Broadcast(TargetData);
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

	//获得玩家当前画面使用的视角原点和朝向
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
