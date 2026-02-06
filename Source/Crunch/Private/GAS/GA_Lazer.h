// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Lazer.generated.h"

/**
 * 
 */


UCLASS()
class CRUNCH_API UGA_Lazer : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	//作为WaitEvent的监听Tag，回调ShootLazer
	static FGameplayTag GetShootTag();
private:
	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float TargetRange=4000;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float DetectionCylinderRadius=30.f;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float TargetingInterval=0.3f ;
	
	UPROPERTY(EditDefaultsOnly,Category="Anim")
	UAnimMontage* LazerMontage;

	UPROPERTY(EditDefaultsOnly,Category="Effect")
	TSubclassOf<UGameplayEffect> OnGoingConsumptionEffect;

	UPROPERTY(EditDefaultsOnly,Category="Effect")
	TSubclassOf<UGameplayEffect> HitDamageEffect;

	UPROPERTY(EditDefaultsOnly,Category="Effect")
	float HitPushSpeed=3000.f;

	FActiveGameplayEffectHandle OnGoingConsumptionEffectHandle;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	TSubclassOf<class ATargetActor_Line> LazerTargetActorClass;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	FName TargetActorAttachSocketName="Lazer";
	
	//在ShootTag被Montage触发时调用，监听Mana变化并应用ConsumeGE
	UFUNCTION()
	void ShootLazer(FGameplayEventData PayLoad);

	//GetGameplayAttributeValueChangeDelegate的委托回调，监听Mana值，不足以Commit时EndAbility
	void ManaUpdated(const FOnAttributeChangeData& Data);

	//对TargetDataHandle的ActorArray应用GE，施加PushTarget
	UFUNCTION()
	void TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
};
