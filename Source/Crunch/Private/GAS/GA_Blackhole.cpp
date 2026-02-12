// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Blackhole.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "TargetActor_GrounPick.h"
#include "UCAbilitySystemStatics.h"
#include "GAS/TA_Blackhole.h"
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
	RemoveFocusEffect();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Blackhole::PlaceBlackhole(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return ;
	}

	RemoveAimEffect();
	AddFocusEffect();
	
	if (PlayCastBlackholeMontageTask)
	{
		PlayCastBlackholeMontageTask->OnBlendOut.RemoveAll(this);
		PlayCastBlackholeMontageTask->OnCancelled.RemoveAll(this);
		PlayCastBlackholeMontageTask->OnInterrupted.RemoveAll(this);
		PlayCastBlackholeMontageTask->OnCompleted.RemoveAll(this);
	}

	if (HasAuthorityOrPredictionKey(CurrentActorInfo,&CurrentActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayHoldBlackholeMontageTask=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,HoldBlackholeMontage);
		PlayHoldBlackholeMontageTask->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayHoldBlackholeMontageTask->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayHoldBlackholeMontageTask->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayHoldBlackholeMontageTask->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayHoldBlackholeMontageTask->ReadyForActivation();
	}

	BlackholeTargetingTask = UAbilityTask_WaitTargetData::WaitTargetData(this,NAME_None,EGameplayTargetingConfirmation::UserConfirmed,BlackholeTargetActorClass);
	BlackholeTargetingTask->ValidData.AddDynamic(this,&ThisClass::FinalTargetsReceived);
	BlackholeTargetingTask->Cancelled.AddDynamic(this,&ThisClass::FinalTargetsReceived);
	BlackholeTargetingTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	BlackholeTargetingTask->BeginSpawningActor(this,BlackholeTargetActorClass,TargetActor);
	ATA_Blackhole* BlackholeTargetActor=Cast<ATA_Blackhole>(TargetActor);
	if (BlackholeTargetActor)
	{
		BlackholeTargetActor->ConfigureBlackhole(TargetAreaRadius,BlackholePullSpeed,BlackholeDuration,GetOwnerTeamId());
	}

	BlackholeTargetingTask->FinishSpawningActor(this, TargetActor);
	if (BlackholeTargetActor)
	{
		BlackholeTargetActor->SetActorLocation(UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle,1).ImpactPoint);
	}
}

void UGA_Blackhole::PlacementCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	K2_EndAbility();
}
 
void UGA_Blackhole::FinalTargetsReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	FGameplayCueParameters FinalBlowCueParams;
	FinalBlowCueParams.Location = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle,1).ImpactPoint;
	FinalBlowCueParams.RawMagnitude = TargetAreaRadius ;
	
	if (K2_HasAuthority())
	{
		BP_ApplyGameplayEffectToTarget(TargetDataHandle,FinalBlowDamageEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));

		FVector BlowCenter = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle,1).ImpactPoint;

		PushTargetsFromLocation(TargetDataHandle,BlowCenter,BlowPushSpeed);

		UAbilityTask_PlayMontageAndWait* PlayFinalBlowMontageTask=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,HoldBlackholeMontage);
		PlayFinalBlowMontageTask->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayFinalBlowMontageTask->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayFinalBlowMontageTask->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayFinalBlowMontageTask->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayFinalBlowMontageTask->ReadyForActivation();
	}
	else
	{
		PlayMontageLocally(FinalBlowMontage);
		
		GetAbilitySystemComponentFromActorInfo() -> ExecuteGameplayCue(UCAbilitySystemStatics::GetCameraShakeCueTag() , FinalBlowCueParams);
	}
	
	GetAbilitySystemComponentFromActorInfo() -> ExecuteGameplayCue(FinalBlowCueTag , FinalBlowCueParams);
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

void UGA_Blackhole::AddFocusEffect()
{
	FocusEffectHandle=BP_ApplyGameplayEffectToOwner(FocusEffect);
}

void UGA_Blackhole::RemoveFocusEffect()
{
	if (FocusEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(FocusEffectHandle);
	}
}
