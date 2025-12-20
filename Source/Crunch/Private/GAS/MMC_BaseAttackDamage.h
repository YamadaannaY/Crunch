// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_BaseAttackDamage.generated.h"

/**
 * 
 */
UCLASS()
class UMMC_BaseAttackDamage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	//配置属性值的Def
	UMMC_BaseAttackDamage();

	//计算逻辑，返回一个最终值
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec)  const override;

private:
	//Def
	
	FGameplayEffectAttributeCaptureDefinition DamageCaptureDef;
	FGameplayEffectAttributeCaptureDefinition ArmorCaptureDef;
};
