// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Combo.h"
#include "GameplayTagsManager.h"
#include "UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"


UGA_Combo::UGA_Combo()
{
	//为了在GA内不重复触发GA
	AbilityTags.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (! K2_CommitAbility())
	{
		K2_EndAbility();
		return ;
	}
	
	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		//Task服务端执行并在这个角色所在所有客户端调用，PredictionKey检测能力是否可预测，如果是不等待服务端直接执行
		UAbilityTask_PlayMontageAndWait* PlayComboMontageTask=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,ComboMontage);
		PlayComboMontageTask->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayComboMontageTask->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayComboMontageTask->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayComboMontageTask->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayComboMontageTask->ReadyForActivation();

		//接收Notify的EventTag
		UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,GetComboChangedEventTag(),nullptr,false,false);
		WaitComboChangeEventTask->EventReceived.AddDynamic(this,&ThisClass::ComboChangedEventReceived);
		WaitComboChangeEventTask->ReadyForActivation();
	}

	//处理第一次输入
	SetupWaitComboInputPress();
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.change");
}

FGameplayTag UGA_Combo::GetComboChangedEventEndTag()
{
	return FGameplayTag::RequestGameplayTag("ability.combo.change.end");
}

void UGA_Combo::SetupWaitComboInputPress()
{
	UAbilityTask_WaitInputPress* WaitInputPress=UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this,&ThisClass::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void UGA_Combo::HandleInputPress(float TimeWaited)
{
	SetupWaitComboInputPress();
	TryCommitCombo();
}

void UGA_Combo::TryCommitCombo()
{
	if (NextComboName==NAME_None) return;
	
	UAnimInstance* OwnerAnimInst=GetOwnerAnimInstance();
	if (!OwnerAnimInst) return;

	//设置当前Montage Section的NextSection
	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(ComboMontage),NextComboName,ComboMontage);
}

void UGA_Combo::ComboChangedEventReceived(FGameplayEventData InPayLoad)
{
	const FGameplayTag EventTag=InPayLoad.EventTag;

	if (EventTag==GetComboChangedEventEndTag())
	{
		//此时到Section末尾，需要重置NextComboName（因为逻辑上设置只要不是空就执行Name对应的Section）
		NextComboName=NAME_None;
		UE_LOG(LogTemp,Warning,TEXT("next combo is cleared"));

		return;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	NextComboName=TagNames.Last();

	UE_LOG(LogTemp,Warning,TEXT("next combo is now :%s"),*NextComboName.ToString());
}
