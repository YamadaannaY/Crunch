//返回MMC基础伤害数值

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
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec)  const override;

private:
	//Def
	FGameplayEffectAttributeCaptureDefinition DamageCaptureDef;
	FGameplayEffectAttributeCaptureDefinition ArmorCaptureDef;
};
