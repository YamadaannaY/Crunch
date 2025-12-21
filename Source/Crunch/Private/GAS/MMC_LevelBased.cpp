// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC_LevelBased.h"
#include "CHeroAttributeSet.h"

UMMC_LevelBased::UMMC_LevelBased()
{

	//获取Level值
	LevelCaptureDefinition.AttributeSource=EGameplayEffectAttributeCaptureSource::Target;
	LevelCaptureDefinition.AttributeToCapture=UCHeroAttributeSet::GetLevelAttribute();

	RelevantAttributesToCapture.Add(LevelCaptureDefinition);
}

float UMMC_LevelBased::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	UAbilitySystemComponent* ASC=Spec.GetContext().GetInstigatorAbilitySystemComponent();

	if (!ASC) return 0.0f;

	float Level=0.f;
	
	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags=Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParams.TargetTags=Spec.CapturedTargetTags.GetAggregatedTags();

	GetCapturedAttributeMagnitude(LevelCaptureDefinition,Spec,EvaluateParams,Level);
	
	bool bFound=false;
	const float RateAttributeVal=ASC->GetGameplayAttributeValue(RateAttribute,bFound);

	if (!bFound) return 0.0f;

	//等级越高此数值越高，增加的Health和Mana也线性增长
	return (Level-1)* RateAttributeVal;
}
