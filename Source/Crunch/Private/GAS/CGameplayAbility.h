// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	//通过MeshComp获得AnimInstance
	UAnimInstance* GetOwnerAnimInstance() const;

	//以AN中记录的起始点和结束点为参数进行SphereTrace
	TArray<FHitResult> GetHitResultsFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle,float SphereSweepRadius=30.f,bool bShowDebug=false,bool bIgnoreSelf=true);
};
