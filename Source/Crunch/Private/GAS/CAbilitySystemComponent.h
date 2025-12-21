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

	//找到DT中对应ASC拥有者的Class并获取其数据，为属性集赋值
	void InitializeBaseAttribute();

	//在Controller的Possess函数中调用，即在服务端实现的Init操作
	void ServerSideInit();

	//遍历InitGE并调用
	void ApplyFullStatsEffect();

	//获得Character特有的GA，不包括BasicGA
	const TMap<ECAbilityInputID,TSubclassOf<UGameplayAbility>>& GetAbilities() const ;

	//判断当前是否处于MaxLevel
	bool IsAtMaxLevel() const;
private:
	//应用初始GE
	void ApplyInitialEffects();

	//对所有GA调用GiveAbility注册到ASC
	void GiveInitialAbilities();

	//将GE应用给自身的调用函数
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect>GameplayEffect,int Level=1);

	//监听Health、Mana属性值，变化时调用函数
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void ManaUpdated(const FOnAttributeChangeData& ChangeData);
	void ExperienceUpdated(const FOnAttributeChangeData& ChangeData);
	
	/***********	GA会带有一个ID，激活能力时依据ID找到要激活的GA，其中PassiveGA不需要ID，直接用None，表示不能主动触发	****************/

	//技能
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Abilities")
	TMap<ECAbilityInputID,TSubclassOf<UGameplayAbility>> Abilities;

	//角色的ASC直接具有的基础能力
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Abilities")
	TMap<ECAbilityInputID,TSubclassOf<UGameplayAbility>> BasicAbilities;

	UPROPERTY(EditDefaultsOnly,Category="Gameplay Abilities")
	class UPA_AbilitySystemGeneric* AbilitySystemGeneric;
};
