//等级越高此MMC数值计算结果越高

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_LevelBased.generated.h"

/**
 * 
 */
UCLASS()
class UMMC_LevelBased : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_LevelBased();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	//Rate属性值
	UPROPERTY(EditDefaultsOnly)
	FGameplayAttribute RateAttribute;
	
	FGameplayEffectAttributeCaptureDefinition LevelCaptureDefinition;
};
