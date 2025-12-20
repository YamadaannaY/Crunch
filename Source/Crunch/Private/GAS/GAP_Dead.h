// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GAP_Dead.generated.h"

/**
 * 
 */
UCLASS()
class UGAP_Dead : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	//配置触发条件
	UGAP_Dead();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	//判定Reward的范围
	UPROPERTY(EditDefaultsOnly,Category="Reward")
	float RewardRange=1000.f;

	//获取范围内对象
	TArray<AActor*> GetRewardTargets() const ;
};
