// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Dash.h"
#include "ATargetActor_Around.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility() || !DashMontage)
	{
		K2_EndAbility();
		return ;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* DashMontagePlay = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,DashMontage);
		DashMontagePlay->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
		DashMontagePlay->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
		DashMontagePlay->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
		DashMontagePlay->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
		DashMontagePlay->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitDashStart = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,GetDashStartTag());
		WaitDashStart->EventReceived.AddDynamic(this,&ThisClass::StartDashing);
		WaitDashStart->ReadyForActivation();
	}
	
}

void UGA_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent*  ASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	if (ASC && DashEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(DashEffectHandle);
	}

	if (DashForwardTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DashForwardTimerHandle);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Dash::StartDashing(FGameplayEventData PayLoad)
{
	if (K2_HasAuthority())
	{
		if (DashEffect)
		{
			DashEffectHandle=BP_ApplyGameplayEffectToOwner(DashEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
		}

		if (IsLocallyControlled())
		{
			DashForwardTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this,&ThisClass::DashForward);
			OwnerCharacterMovementComponent=GetAvatarActorFromActorInfo()->GetComponentByClass<UCharacterMovementComponent>();
		}
	}

		UAbilityTask_WaitTargetData* WaitDashAroundTargetData =UAbilityTask_WaitTargetData::WaitTargetData(this,NAME_None,EGameplayTargetingConfirmation::CustomMulti,TargetActorClass);
		WaitDashAroundTargetData->ValidData.AddDynamic(this,&ThisClass::TargetReceived);
		WaitDashAroundTargetData->ReadyForActivation();

		AGameplayAbilityTargetActor* TargetActor;
		WaitDashAroundTargetData->BeginSpawningActor(this,TargetActorClass,TargetActor);
		AATargetActor_Around* TargetAroundActor=Cast<AATargetActor_Around>(TargetActor);
		if (TargetAroundActor)
		{
			TargetAroundActor->ConfigureAroundActor(TargetDetectionRange,GetOwnerTeamId(),LocalGameplayCueTag);
		}

		WaitDashAroundTargetData->FinishSpawningActor(this,TargetActor);

		if (TargetAroundActor)
		{
			TargetAroundActor->AttachToComponent(GetOwningComponentFromActorInfo(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetActorAttachSocketName);
		}
}

void UGA_Dash::TargetReceived(const FGameplayAbilityTargetDataHandle&  Data)
{
	if (K2_HasAuthority())
	{
		if (DamageEffect)
		{
			BP_ApplyGameplayEffectToTarget(Data,DamageEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
			PushTargetsFromOwningLocation(Data,HitPushSpeed);
		}
	}
}

void UGA_Dash::DashForward()
{
	if (OwnerCharacterMovementComponent)
	{
		FVector ForwardVector = GetAvatarActorFromActorInfo()->GetActorForwardVector();
		OwnerCharacterMovementComponent->AddInputVector(ForwardVector);
		
		DashForwardTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this,&ThisClass::DashForward);
	}
}