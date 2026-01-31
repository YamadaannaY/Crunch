// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/GA_Shoot.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Crunch/DebugHelper.h"
#include "GAS/ProjectileActor.h"
#include "GameplayTagsManager.h"


UGA_Shoot::UGA_Shoot() : ShootMontage(nullptr),AimTarget(nullptr),AimTargetAbilitySystemComponent(nullptr)
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

void UGA_Shoot::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AimTargetAbilitySystemComponent)
	{
		AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).RemoveAll(this);
		AimTargetAbilitySystemComponent=nullptr;
	}

	StopShooting(FGameplayEventData());

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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


void UGA_Shoot::StartAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(AimTargetCheckTimerHandle,this,&ThisClass::FindAimTarget,AimTargetCheckTimeInterval,true);
	}
}

void UGA_Shoot::StopAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AimTargetCheckTimerHandle);
	}
}

bool UGA_Shoot::HasValidTarget() const 
{
	if (!AimTarget) return false;
	if (UCAbilitySystemStatics::IsActorDead(AimTarget)) return false;
	if (!IsTargetInRange()) return false;

	return true;
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
	FindAimTarget();
	StartAimTargetCheckTimer();
}

void UGA_Shoot::StopShooting(FGameplayEventData PayLoad)
{
	Debug::Print("stop shooting");

	if (ShootMontage)
	{
		StopMontageAfterCurrentSection(ShootMontage);
	}
	StopAimTargetCheckTimer();
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
			Projectile->ShootProjectile(ShootProjectileSpeed,ShootProjectileRange,GetAimTargetIfValid(),GetOwnerTeamId(),MakeOutgoingGameplayEffectSpec(ProjectileEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo)));
		}
	}
}

AActor* UGA_Shoot::GetAimTargetIfValid() const
{
	if (HasValidTarget())
	{
		return AimTarget;
	}
	return nullptr;
}

void UGA_Shoot::FindAimTarget()
{
	if (!HasValidTarget()) return ;

	if (AimTargetAbilitySystemComponent)
	{
		//每次调用都重新寻找AimTarget，需要重置ASC
		AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).RemoveAll(this);
		AimTargetAbilitySystemComponent=nullptr;
	}

	AimTarget=GetAimTarget(ShootProjectileRange,ETeamAttitude::Hostile);
	if (AimTarget)
	{
		AimTargetAbilitySystemComponent=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AimTarget);
		if (AimTargetAbilitySystemComponent)
		{
			AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this,&ThisClass::TargetDeadTagUpdated);
		}
	}
}

void UGA_Shoot::TargetDeadTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		FindAimTarget();
	}
}

bool UGA_Shoot::IsTargetInRange() const
{
	if (!AimTarget) return false;

	float Dist=FVector::Distance(AimTarget->GetActorLocation(),GetAvatarActorFromActorInfo()->GetActorLocation());

	return Dist<=ShootProjectileRange;
}
