// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GA_Shoot.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Shoot : public UCGameplayAbility
{
	GENERATED_BODY()

	UGA_Shoot();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
private:
	UPROPERTY(EditDefaultsOnly,Category="Anim")
	UAnimMontage* ShootMontage;

	UPROPERTY(EditDefaultsOnly,Category="Shoot")
	float ShootProjectileSpeed=2000.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Shoot")
	float ShootProjectileRange=3000.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Shoot")
	TSubclassOf<class AProjectileActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly,Category="Shoot")
	TSubclassOf<UGameplayEffect> ProjectileEffect;	
	
	static FGameplayTag GetShootTag();
	
	UFUNCTION()
	void StartShooting(FGameplayEventData PayLoad);
	
	UFUNCTION()
	void StopShooting(FGameplayEventData PayLoad);

	UFUNCTION()
	void ShootProjectile(FGameplayEventData PayLoad);

	//获取AimTarget
	AActor* GetAimTargetIfValid() const;
};
