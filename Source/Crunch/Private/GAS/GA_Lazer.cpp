// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Lazer.h"
#include "AbilitySystemComponent.h"
#include "CAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitCancel.h"

void UGA_Lazer::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility() || !LazerMontage) return ;
	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayerLazerMontageTask=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,LazerMontage);
		PlayerLazerMontageTask->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayerLazerMontageTask->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayerLazerMontageTask->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayerLazerMontageTask->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayerLazerMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootEvent=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,GetShootTag());
		WaitShootEvent->EventReceived.AddDynamic(this,&ThisClass::ShootLazer);
		WaitShootEvent->ReadyForActivation();

		UAbilityTask_WaitCancel* WaitCancel=UAbilityTask_WaitCancel::WaitCancel(this);
		WaitCancel->OnCancel.AddDynamic(this,&ThisClass::K2_EndAbility);
		WaitCancel->ReadyForActivation();
	}
}

void UGA_Lazer::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* OwnerASC=GetAbilitySystemComponentFromActorInfo();
	if (OwnerASC && OnGoingConsumptionEffectHandle.IsValid())
	{
		OwnerASC->RemoveActiveGameplayEffect(OnGoingConsumptionEffectHandle);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_Lazer::GetShootTag()
{
	return FGameplayTag::RequestGameplayTag("ability.lazer.shoot");
}

void UGA_Lazer::ShootLazer(FGameplayEventData PayLoad)
{
	if (K2_HasAuthority())
	{
		OnGoingConsumptionEffectHandle=BP_ApplyGameplayEffectToOwner(OnGoingConsumptionEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
		UAbilitySystemComponent* OwnerASC=GetAbilitySystemComponentFromActorInfo();
		if (OwnerASC)
		{
			OwnerASC->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this,&ThisClass::ManaUpdated);
		}
	}
}

void UGA_Lazer::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	UAbilitySystemComponent* OwnerASC=GetAbilitySystemComponentFromActorInfo();
	if (OwnerASC && !OwnerASC->CanApplyAttributeModifiers(OnGoingConsumptionEffect.GetDefaultObject(),GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo),MakeEffectContext(CurrentSpecHandle,CurrentActorInfo)))
	{
		K2_EndAbility();
	}
}