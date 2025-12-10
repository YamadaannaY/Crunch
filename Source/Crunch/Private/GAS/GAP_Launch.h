// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GAP_Launch.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UGAP_Launch : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	//在构造函数中使用TriggerData确定了这个PassiveGA的触发条件，以GameplayEvent+Tag的方式
	UGAP_Launch();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//工具函数：获得激活这个GA的Tag
	static FGameplayTag GetLaunchedAbilityActivationTag();
};
