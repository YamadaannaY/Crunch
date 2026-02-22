// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Dash.generated.h"

class UCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class CRUNCH_API UGA_Dash : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	static FGameplayTag GetDashStartTag() {return FGameplayTag::RequestGameplayTag("ability.dash.start");}
private:
	UPROPERTY(EditDefaultsOnly,Category="Montage")
	UAnimMontage* DashMontage;

	UPROPERTY(EditDefaultsOnly,Category="Effects")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly,Category="Effects")
	TSubclassOf<UGameplayEffect> DashEffect;
	
	FActiveGameplayEffectHandle DashEffectHandle;

	UPROPERTY(EditDefaultsOnly,Category="GameplayCue")
	FGameplayTag LocalGameplayCueTag;

	UPROPERTY(EditDefaultsOnly,Category="Effects")
	float HitPushSpeed=3000.f;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float TargetDetectionRange = 300.f;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	TSubclassOf<class  AATargetActor_Around> TargetActorClass;

	//绑定TargetActor的Socket
	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	FName TargetActorAttachSocketName="TargetDashCenter";

	//等待Montage的Tag调用Dash逻辑
	UFUNCTION()
	void StartDashing(FGameplayEventData PayLoad);

	//对TargetActor应用DamageGE
	UFUNCTION()
	void TargetReceived(const FGameplayAbilityTargetDataHandle& Data);

	UPROPERTY()
	UCharacterMovementComponent* OwnerCharacterMovementComponent;

	FTimerHandle DashForwardTimerHandle;

	//具体的Dash逻辑，获取前向向量，定时器递归AddInputVector
	void DashForward();
};
