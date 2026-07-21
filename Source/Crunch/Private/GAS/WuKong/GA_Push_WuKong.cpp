// 悟空突刺：ActivateAbility 播放突刺 Montage → AnimNotify 触发 StartPushPhase →
// 前向 SphereSweep 一次性捕获突刺路径上所有敌对目标，施加 DamageGE + 前向击退
// GA 期间禁止移动和跳跃输入，EndAbility 时恢复
// 伤害检测仅在服务端（Server-Authoritative）

#include "GA_Push_WuKong.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Character/CCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"

class ACCharacter;

UGA_Push_WuKong::UGA_Push_WuKong()
{
	SetAssetTags(FGameplayTagContainer(GetPushAbilityTag()));
	BlockAbilitiesWithTag.AddTag(GetPushAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_Push_WuKong::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility() || !PushMontage)
	{
		K2_EndAbility();
		return;
	}
	
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, PushMontage);
		PlayTask->OnBlendOut.AddDynamic(this, &ThisClass::K2_EndAbility);
		PlayTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
		PlayTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
		PlayTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
		PlayTask->ReadyForActivation();
		
		UAbilityTask_WaitGameplayEvent* WaitBlockMove= UAbilityTask_WaitGameplayEvent::WaitGameplayEvent
		(this,FGameplayTag::RequestGameplayTag(TEXT("ability.block.move")));
		WaitBlockMove->EventReceived.AddDynamic(this,&ThisClass::BlockMoveInput);
		WaitBlockMove->ReadyForActivation();
		UAbilityTask_WaitGameplayEvent* WaitEnableMove= UAbilityTask_WaitGameplayEvent::WaitGameplayEvent
		(this,FGameplayTag::RequestGameplayTag(TEXT("ability.enable.move")));
		WaitEnableMove->EventReceived.AddDynamic(this,&ThisClass::EnableMoveInput);
		WaitEnableMove->ReadyForActivation();
		
		StartPushPhase();
	}
}

void UGA_Push_WuKong::StartPushPhase()
{
	if (!K2_HasAuthority()) return;

	AActor* OwnerAvatar = GetAvatarActorFromActorInfo();
	if (!OwnerAvatar) return;

	const FVector SweepStart = OwnerAvatar->GetActorLocation();
	const FVector ForwardDir = OwnerAvatar->GetActorForwardVector();
	const FVector SweepEnd = SweepStart + ForwardDir * ThrustSweepDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerAvatar);

	TArray<FHitResult> HitResults;
	GetWorld()->SweepMultiByChannel(
		HitResults,
		SweepStart, SweepEnd,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(SweepRadius),
		QueryParams);

	for (const FHitResult& Hit : HitResults)
	{
		if (!IsActorTeamAttitudeIs(Hit.GetActor(), ETeamAttitude::Hostile)) continue;
		if (UCAbilitySystemStatics::IsActorDead(Hit.GetActor())) continue;

		// DamageGE
		if (PushDamageEffectDef.DamageEffect)
		{
			FHitResult DamageHit(
				Hit.GetActor(),
				Cast<UPrimitiveComponent>(Hit.GetActor()->GetRootComponent()),
				Hit.GetActor()->GetActorLocation(),
				-ForwardDir);
			ApplyGameplayEffectToHitResultActor(DamageHit, PushDamageEffectDef.DamageEffect,
				GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}

		// 击退：X前向 + Z向上
		const FVector PushVel = ForwardDir * PushDamageEffectDef.PushVelocity.X +FVector::UpVector * PushDamageEffectDef.PushVelocity.Z;
		if (!PushVel.IsNearlyZero())
		{
			PushTarget(Hit.GetActor(), PushVel);
		}
	}
}

void UGA_Push_WuKong::EnableMoveInput(FGameplayEventData PayLoad)
{
	// 恢复突刺前的移动模式
	if (ACCharacter* Character = Cast<ACCharacter>(GetOwningAvatarCharacter()))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->SetMovementMode(CachedMovementMode);
		}
	}
}

void UGA_Push_WuKong::BlockMoveInput(FGameplayEventData PayLoad)
{
	// 突刺期间禁止移动和跳跃
	if (ACCharacter* Character = Cast<ACCharacter>(GetOwningAvatarCharacter()))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			CachedMovementMode = MovementComp->MovementMode;
			MovementComp->SetMovementMode(MOVE_None);
		}
	}
}

void UGA_Push_WuKong::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                 const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
