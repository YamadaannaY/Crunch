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
	UPROPERTY(EditDefaultsOnly,Category="Anim")
	UAnimMontage* LazerMontage;

	UPROPERTY(EditDefaultsOnly,Category="Effect")
	TSubclassOf<UGameplayEffect> OnGoingConsumptionEffect;

	FActiveGameplayEffectHandle OnGoingConsumptionEffectHandle;

	//在ShootTag被Montage触发时调用，监听Mana变化并应用ConsumeGE
	UFUNCTION()
	void ShootLazer(FGameplayEventData PayLoad);

	//GetGameplayAttributeValueChangeDelegate的委托回调，监听Mana值，不足以Commit时EndAbility
	void ManaUpdated(const FOnAttributeChangeData& Data);
};
