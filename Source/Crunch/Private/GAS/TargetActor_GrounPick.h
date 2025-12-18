// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_GrounPick.generated.h"

/**
 * 
 */
UCLASS()
class ATargetActor_GroundPick : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	ATargetActor_GroundPick();
protected:
	virtual void Tick(float DeltaSeconds) override;

	//设置TargetActor的位置
	FVector GetTargetPoint()const ;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float TargetTraceRange=2000.f;
};
