// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbilitiesType.h"
#include "GAS/CGameplayAbility.h"
#include "UpperCut.generated.h"

/**
 * 
 */
UCLASS()
class UUpperCut : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UUpperCut();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	//存储SectionName下对应的GE+PushVel
	UPROPERTY(EditDefaultsOnly,Category="Launch")
	TMap<FName,FGenericDamageEffectDef> ComboDamageMap;

	//UpperCut第一段MontageSection的DamageGE
	UPROPERTY(EditDefaultsOnly,Category="Launch")
	TSubclassOf<UGameplayEffect> LaunchDamageEffect;

	//击飞速度
	UPROPERTY(EditDefaultsOnly,Category="Launch")
	float UpperCutLaunchSpeed=1000.f;

	//击飞期间连招下的击飞速度
	UPROPERTY(EditDefaultsOnly,Category="Launch")
	float UpperComboHoldSpeed=100.f;	
	
	//上勾拳动画
	UPROPERTY(EditDefaultsOnly,Category="Animation")
	UAnimMontage* UpperCutMontage;

	//获取击飞Tag
	static FGameplayTag GetUpperCutLaunchTag();

	//根据当前SectionName找到对应的DamageGE+PushVel
	const FGenericDamageEffectDef* GetDamageEffectDefForCurrentCombo() const ;

	//具体Launch逻辑
	UFUNCTION()
	void StartLaunching(FGameplayEventData EventData);

	//查询当前Notify的Tag，并将对应Tag的后缀为NextComboName赋值
	UFUNCTION()
	void HandleComboChangeEvent(FGameplayEventData EventData);

	//如果在当前Section触发了BasicattackPressTag，则调用此函数处理NextComboName赋值结果，根据值设置NextSection
	UFUNCTION()
	void HandleComboCommitEvent(FGameplayEventData EventData);

	//Montage中Notify带有DamageTag则调用这个函数，逻辑和GACombo完全一致，多了PushVel对目标施加作用力
	UFUNCTION()
	void HandleComboDamageEvent(FGameplayEventData EventData);
	
	FName NextComboName;
};
