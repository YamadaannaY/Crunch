// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Shoot.h"

#include "CAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Crunch/DebugHelper.h"

UGA_Shoot::UGA_Shoot() : ShootMontage(nullptr)
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
}

void UGA_Shoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return ;
	}
	Debug::Print("shoot activated");
	
	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		UAbilityTask_WaitGameplayEvent* WaitStartShootingTask=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UCAbilitySystemStatics::GetBasicAttackInputPressedTag());
		WaitStartShootingTask->EventReceived.AddDynamic(this,&ThisClass::StartShooting);
		WaitStartShootingTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitStopShootingTask=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UCAbilitySystemStatics::GetBasicAttackInputReleasedTag());
		WaitStopShootingTask->EventReceived.AddDynamic(this,&ThisClass::StopShooting);
		WaitStopShootingTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootProjectileEvent=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,GetShootTag(),nullptr,false,false);
		WaitShootProjectileEvent->EventReceived.AddDynamic(this,&ThisClass::ShootProjectile);
		WaitShootProjectileEvent->ReadyForActivation();
	}
}

void UGA_Shoot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Debug::Print("shoot ability ended");
	K2_EndAbility();
}

FGameplayTag UGA_Shoot::GetShootTag()
{
	return FGameplayTag::RequestGameplayTag("ability.shoot");
}

void UGA_Shoot::StartShooting(FGameplayEventData PayLoad)
{
	Debug::Print("start shooting");
	if (HasAuthority(&CurrentActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayShootMontage=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,ShootMontage);
		PlayShootMontage->ReadyForActivation();
	}
	else
	{
		PlayMontageLocally(ShootMontage);
	}
}

void UGA_Shoot::StopShooting(FGameplayEventData PayLoad)
{
	Debug::Print("stop shooting");

	if (ShootMontage)
	{
		StopMontageAfterCurrentSection(ShootMontage);
	}
}

void UGA_Shoot::ShootProjectile(FGameplayEventData PayLoad)
{
	Debug::Print("shooting projectile");
}
