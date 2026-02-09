// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Blackhole.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UGA_Blackhole : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float TargetAreaRadius=1000.f;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float BlackholePullSpeed=3000.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float BlackholeDuration=6.f;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float TargetTraceRange = 2000.f;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	TSubclassOf<class ATargetActor_GroundPick> TargetActorClass;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	TSubclassOf<class ATA_Blackhole> BlackholeTargetActorClass;
	
	UPROPERTY(EditDefaultsOnly,Category="Animation")
	UAnimMontage* TargetingMontage;
	
	UPROPERTY(EditDefaultsOnly,Category="Animation")
	UAnimMontage* HoldBlackholeMontage;

	UPROPERTY(EditDefaultsOnly,Category="Animation")
	UAnimMontage* FinalBlowMontage;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	TSubclassOf<UGameplayEffect> AimEffect;

	FActiveGameplayEffectHandle AimEffectHandle;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	TSubclassOf<UGameplayEffect> FocusEffect;

	FActiveGameplayEffectHandle FocusEffectHandle;

	UPROPERTY(EditDefaultsOnly,Category="Effect")
	TSubclassOf<UGameplayEffect> FinalBlowDamageEffect;

	UPROPERTY(EditDefaultsOnly,Category="Effect")
	float BlowPushSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly,Category="Effect")
	FGameplayTag FinalBlowCueTag;	

	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* PlayCastBlackholeMontageTask;

	UPROPERTY()
	class UAbilityTask_WaitTargetData* BlackholeTargetingTask;

	//释放Blackhole
	UFUNCTION()
	void PlaceBlackhole(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	//取消释放
	UFUNCTION()
	void PlacementCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	//处理接受到的TargetData
	UFUNCTION()
	void FinalTargetsReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	//添加stat.aim Tag
	void AddAimEffect();
	
	//移除stat.aim Tag
	void RemoveAimEffect();
	
	//添加stat.aim Tag
	void AddFocusEffect();
	
	//移除stat.aim Tag
	void RemoveFocusEffect();
	
};
