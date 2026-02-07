// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Blackhole.h"
#include "TargetActor_GrounPick.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

void UGA_Blackhole::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (! HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		K2_EndAbility();
		return ;
	}

	PlayCastBlackholeMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,TargetingMontage);
	PlayCastBlackholeMontageTask->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
	PlayCastBlackholeMontageTask->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
	PlayCastBlackholeMontageTask->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
	PlayCastBlackholeMontageTask->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
	PlayCastBlackholeMontageTask->ReadyForActivation();

	UAbilityTask_WaitTargetData* WaitPlacementTask = UAbilityTask_WaitTargetData::WaitTargetData(this,NAME_None,EGameplayTargetingConfirmation::UserConfirmed,TargetActorClass);
	WaitPlacementTask->ValidData.AddDynamic(this,&ThisClass::PlaceBlackhole);
	WaitPlacementTask->Cancelled.AddDynamic(this,&ThisClass::PlacementCancelled);
	WaitPlacementTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitPlacementTask->BeginSpawningActor(this,TargetActorClass,TargetActor);
	ATargetActor_GroundPick* GroundPickTargetActor=Cast<ATargetActor_GroundPick>(TargetActor);
	if (GroundPickTargetActor)
	{
		GroundPickTargetActor->SetShouldDrawDebug(ShouldDrawDebug());
		GroundPickTargetActor->SetTargetAreaRadius(TargetAreaRadius);
		GroundPickTargetActor->SetTargetTraceRange(TargetTraceRange);
	}

	WaitPlacementTask->FinishSpawningActor(this, TargetActor);

	AddAimEffect();
}

void UGA_Blackhole::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	RemoveAimEffect();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Blackhole::PlaceBlackhole(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	
}

void UGA_Blackhole::PlacementCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	
}

void UGA_Blackhole::AddAimEffect()
{
	AimEffectHandle=BP_ApplyGameplayEffectToOwner(AimEffect);
}

void UGA_Blackhole::RemoveAimEffect()
{
	if (AimEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(AimEffectHandle);
	}
}
