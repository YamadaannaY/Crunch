// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Combo.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UGA_Combo : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Combo();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//获得ComboChange下所有具体的comboTag
	static FGameplayTag GetComboChangedEventTag();
	//获得ComboChange下的endTag
	static FGameplayTag GetComboChangedEventEndTag();
	//获得TargetEvent对应的DamageTag
	static FGameplayTag GetComboTargetEventTag();

private:
	//实现一个WaitInputPressTask，绑定触发输入后的回调HandleInputPress
	void SetupWaitComboInputPress();

	//再次实现WaitInputPressTask处理下一次输入，形成循环，同时处理当前press
	UFUNCTION()
	void HandleInputPress(float TimeWaited);

	//press后，若NextComboName存在，则设置NextSection为这个Name对应的Section
	void TryCommitCombo();
	
	//DamageGE
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Effect")
	TSubclassOf<UGameplayEffect> DefaultDamageEffect;

	//对不同Section对应的Montage触发的DamageGE进行不同的设置
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Effect")
	TMap<FName,TSubclassOf<UGameplayEffect>> DamageEffectMap;

	//找到当前Section对应的DamageGE
	TSubclassOf<UGameplayEffect> GetDamageEffectForCurrentCombo() const ;
	
	//包含所有ComboAnimationSequence的Montage
	UPROPERTY(EditDefaultsOnly,Category="Animation")
	UAnimMontage* ComboMontage;

	//EventReceived的回调函数，找到下一个Tag的后缀，即NextComboName
	UFUNCTION()
	void ComboChangedEventReceived(FGameplayEventData InPayLoad);

	//实现伤害逻辑
	UFUNCTION()
	void DoDamage(FGameplayEventData Data);

	//获得当前ComboMontage对应的下一个ComboMontage的字面量后缀，同时设置ComboSection的字面量和后缀相等
	FName NextComboName;
};
