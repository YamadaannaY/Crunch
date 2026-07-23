#include "GA_Roll_WuKong.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/CCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_Roll_WuKong::UGA_Roll_WuKong()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(GetRollAbilityTag());
	BlockAbilitiesWithTag.AddTag(GetRollAbilityTag());
}

void UGA_Roll_WuKong::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility() || !RollMontage)
	{
		K2_EndAbility();
		return;
	}

	ACCharacter* Character = Cast<ACCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		K2_EndAbility();
		return;
	}

	// Launch：前冲 + 向上，不干预重力，形成类抛物线
	const FVector RollStartDir = GetRollDirection();
	FVector BurstVelocity = RollStartDir * InitialBurstSpeed;
	BurstVelocity.Z = FMath::Max(BurstVelocity.Z, 0.f) + InitialUpwardSpeed;
	
	Character->LaunchCharacter(BurstVelocity, false, false);

	// 给自身附加护盾 GE（Duration=10s，到期自动移除 Shield）
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) && RollShieldEffect)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
				RollShieldEffect, GetAbilityLevel(Handle, ActorInfo), ASC->MakeEffectContext());
			RollShieldEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	StartRollPhase();
}

void UGA_Roll_WuKong::StartRollPhase()
{
	// 每帧前冲（翻滚阶段）
	RollTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::TickRollForward);

	if (HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, RollMontage);
		PlayTask->OnCompleted.AddDynamic(this, &ThisClass::OnRollMontageCompleted);
		PlayTask->OnInterrupted.AddDynamic(this, &ThisClass::OnRollMontageInterrupted);
		PlayTask->OnCancelled.AddDynamic(this, &ThisClass::OnRollMontageInterrupted);
		PlayTask->ReadyForActivation();
	}
}

void UGA_Roll_WuKong::OnRollMontageCompleted()
{
	// 停止前冲，重力自然牵引下落
	if (RollTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(RollTimerHandle);
	}

	// 每帧轮询落地
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::TickCheckLanding);
}

void UGA_Roll_WuKong::OnRollMontageInterrupted()
{
	K2_EndAbility();
}

void UGA_Roll_WuKong::TickCheckLanding()
{
	if (!IsActive()) return;

	const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		K2_EndAbility();
		return;
	}

	if (!Character->GetCharacterMovement()->IsFalling())
	{
		K2_EndAbility();
		return;
	}

	RollTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::TickCheckLanding);
}

void UGA_Roll_WuKong::TickRollForward()
{
	if (!IsActive()) return;

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && RollSpeed > 0.f)
	{
		const FVector MoveDir = GetRollDirection();
		Character->AddMovementInput(MoveDir, RollSpeed / Character->GetCharacterMovement()->MaxWalkSpeed);
	}

	RollTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::TickRollForward);
}

FVector UGA_Roll_WuKong::GetRollDirection() const
{
	const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return FVector::ForwardVector;

	if (bAllowSteering)
	{
		FRotator ControlRot = Character->GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll = 0.f;
		return ControlRot.Vector().GetSafeNormal();
	}

	return Character->GetActorForwardVector();
}

void UGA_Roll_WuKong::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (RollTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(RollTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
