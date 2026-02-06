// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Lazer.h"
#include "AbilitySystemComponent.h"
#include "CAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitCancel.h"
#include "GAS/TargetActor_Line.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

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
		//因为这个GE是Infinite，需要手动Remove
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
	
	UAbilityTask_WaitTargetData* WaitDamageTask=UAbilityTask_WaitTargetData::WaitTargetData(this,NAME_None,EGameplayTargetingConfirmation::CustomMulti,LazerTargetActorClass);
	WaitDamageTask->ValidData.AddDynamic(this,&ThisClass::TargetReceived);
	WaitDamageTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitDamageTask->BeginSpawningActor(this,LazerTargetActorClass,TargetActor);
	
	ATargetActor_Line* LineTargetActor=Cast<ATargetActor_Line>(TargetActor);
	if (LineTargetActor)
	{
		LineTargetActor->ConfigureTargetSetting(TargetRange,DetectionCylinderRadius,TargetingInterval,GetOwnerTeamId(),ShouldDrawDebug());
	}
	
	WaitDamageTask->FinishSpawningActor(this,TargetActor);

	if (LineTargetActor)
	{
		//将Root绑定到Socket
		LineTargetActor->AttachToComponent(GetOwningComponentFromActorInfo(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,TargetActorAttachSocketName);
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

void UGA_Lazer::TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (K2_HasAuthority())
	{
		BP_ApplyGameplayEffectToTarget(TargetDataHandle,HitDamageEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
	}
	
	PushTargets(TargetDataHandle,GetAvatarActorFromActorInfo()->GetActorForwardVector()* HitPushSpeed);
}
