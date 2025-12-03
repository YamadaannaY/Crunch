// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CGameplayAbilitiesType.h"
#include "CAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	//遍历InitGE并调用
	void ApplyInitialEffects();
	void GiveInitialAbilities();

private:
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
