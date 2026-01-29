// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Shoot.h"

#include "CAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Crunch/DebugHelper.h"
#include "GAS/ProjectileActor.h"
#include "GameplayTagsManager.h"


UGA_Shoot::UGA_Shoot() : ShootMontage(nullptr)
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetCrosshairTag());
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
	if (K2_HasAuthority())
	{
		AActor* OwnerAvatarActor=GetAvatarActorFromActorInfo();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner=OwnerAvatarActor;
		SpawnParams.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector SocketLocation=OwnerAvatarActor->GetActorLocation();
		USkeletalMeshComponent* MeshComp=GetOwningComponentFromActorInfo();
		if (MeshComp)
		{
			TArray<FName> OutNames;
			UGameplayTagsManager::Get().SplitGameplayTagFName(PayLoad.EventTag,OutNames);
			if (OutNames.Num()>0)
			{
				FName SocketName=OutNames.Last();
				SocketLocation=MeshComp->GetSocketLocation(SocketName);
			}
		}

		AProjectileActor* Projectile=GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass,SocketLocation,OwnerAvatarActor->GetActorRotation(),SpawnParams);
		if (Projectile)
		{
			Projectile->ShootProjectile(ShootProjectileSpeed,ShootProjectileRange,nullptr,GetOwnerTeamId(),MakeOutgoingGameplayEffectSpec(ProjectileEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo)));
		}
	}
}
