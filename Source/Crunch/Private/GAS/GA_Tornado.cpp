// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Tornado.h"
	
#include "UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitCancel.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"


void UGA_Tornado::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayTornadoMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,TornadoMontage);
		PlayTornadoMontageTask->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayTornadoMontageTask->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayTornadoMontageTask->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayTornadoMontageTask->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayTornadoMontageTask->ReadyForActivation();

		if (K2_HasAuthority())
		{
			UAbilityTask_WaitGameplayEvent* WaitDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UCAbilitySystemStatics::GetGenericDamagePointTag(),nullptr,false);
			WaitDamageEvent->EventReceived.AddDynamic(this,&ThisClass::TornadoDamageEventReceived);
			WaitDamageEvent->ReadyForActivation();
		}

		UAbilityTask_WaitCancel* WaitTornadoCancelTask = UAbilityTask_WaitCancel::WaitCancel(this);
		WaitTornadoCancelTask->OnCancel.AddDynamic(this,&ThisClass::K2_EndAbility);
		WaitTornadoCancelTask->ReadyForActivation();

		UAbilityTask_WaitDelay* WaitTornadoDelayTask = UAbilityTask_WaitDelay::WaitDelay(this,TornadoDuration);
		WaitTornadoDelayTask->OnFinish.AddDynamic(this,&ThisClass::K2_EndAbility);
		WaitTornadoDelayTask->ReadyForActivation();
	}
}

void UGA_Tornado::TornadoDamageEventReceived(FGameplayEventData PayLoad)
{
	if (K2_HasAuthority())
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle = PayLoad.TargetData;
		BP_ApplyGameplayEffectToTarget(TargetDataHandle,TornadoDamageEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));

		PushTargetsFromOwningLocation(TargetDataHandle,HitPushSpeed);
	}
}
