// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Combo.h"

#include "UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UGA_Combo::UGA_Combo()
{
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
	}
}
