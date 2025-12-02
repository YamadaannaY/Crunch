// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

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
		//Task会在这个角色所在所有客户端执行，PredictionKey可以让角色直接预测执行
		UAbilityTask_PlayMontageAndWait* PlayComboMontageTask=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,ComboMontage);
		PlayComboMontageTask->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayComboMontageTask->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayComboMontageTask->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayComboMontageTask->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayComboMontageTask->ReadyForActivation();
	}
}
