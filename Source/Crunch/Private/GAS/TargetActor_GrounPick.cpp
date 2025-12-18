// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/TargetActor_GrounPick.h"
#include "Crunch/Crunch.h"

ATargetActor_GroundPick::ATargetActor_GroundPick()
{
	PrimaryActorTick.bCanEverTick = true;
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
	//还没有找到就保持不动
	if (!TraceResult.bBlockingHit)
	{
		return GetActorLocation();
	}

	return TraceResult.ImpactPoint;
}
