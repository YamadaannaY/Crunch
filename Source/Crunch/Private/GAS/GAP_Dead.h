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

	//基础Gold奖励
	UPROPERTY(EditDefaultsOnly,Category="Reward")
	float BaseGoldReward=200.f;

	//基础Experience奖励
	UPROPERTY(EditDefaultsOnly,Category="Reward")
	float BaseExperienceReward=200.f;

	//被击杀者Experience值参与Experience结算的计算系数
	UPROPERTY(EditDefaultsOnly,Category="Reward")
	float ExperienceRewardPerExperience=0.1f;

	//被击杀者Experience值参与Gold结算的计算系数
	UPROPERTY(EditDefaultsOnly,Category="Reward")
	float GoldRewardPerExperience=0.05f;

	//击杀者获取Reward占比
	UPROPERTY(EditDefaultsOnly,Category="Reward")
	float KillerRewardPortion=0.5f;
	
	//获取范围内对象
	TArray<AActor*> GetRewardTargets() const ;

	UPROPERTY(EditDefaultsOnly,Category="Reward")
	TSubclassOf<UGameplayEffect> RewardEffect;
};
