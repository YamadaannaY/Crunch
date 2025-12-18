// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_GroundBlast.generated.h"

class ATargetActor_GroundPick;
/**
 * 
 */
UCLASS()
class CRUNCH_API UGA_GroundBlast : public UCGameplayAbility
{
	GENERATED_BODY()

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UGA_GroundBlast();
private:
	UPROPERTY(EditDefaultsOnly,Category="Animation")
	UAnimMontage* GroundBlastMontage;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	TSubclassOf<ATargetActor_GroundPick> TargetActorClass;

	//确认释放技能
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Handle);

	//取消技能释放
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& Handle);
};
