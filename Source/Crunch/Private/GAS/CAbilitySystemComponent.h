// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CGameplayAbilitiesType.h"
#include "GameplayEffectTypes.h"
#include "CAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UCAbilitySystemComponent();
	
	//遍历InitGE并调用
	void ApplyInitialEffects();
	void GiveInitialAbilities();

	void ApplyFullStatsEffect();

private:
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect>GameplayEffect,int Level=1);
	//监听Health属性值，变化时调用函数，处理死亡逻辑
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);

	UPROPERTY(EditDefaultsOnly,Category="Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatEffect;

	UPROPERTY(EditDefaultsOnly,Category="Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;
	
	//对属性集所有初始属性的InitGE
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	//技能
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Abilities")
	TMap<ECAbilityInputID,TSubclassOf<UGameplayAbility>> Abilities;

	//角色的ASC直接具有的基础能力
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Abilities")
	TMap<ECAbilityInputID,TSubclassOf<UGameplayAbility>> BasicAbilities;
};
