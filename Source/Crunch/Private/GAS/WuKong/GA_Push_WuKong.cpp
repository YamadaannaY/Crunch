// 悟空突刺：ActivateAbility 播放突刺 Montage → AnimNotify 触发 StartPushPhase →
// 播放前做前向锥形检测，若发现敌对目标则通过 Timer + RInterpTo 平滑转向；
// 未发现目标则直接释放。前向 SphereSweep 一次性捕获突刺路径上所有敌对目标，
// 施加 DamageGE + 前向击退。GA 期间禁止移动和跳跃输入，EndAbility 时恢复。
// 伤害检测仅在服务端（Server-Authoritative）

#include "GA_Push_WuKong.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Character/CCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "AbilitySystemComponent.h"


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
       // 锥形检测：在前向 ±WarpConeHalfAngle° 范围内搜索敌对目标
       // 若找到目标则启动 Timer 平滑旋转角色朝向目标
       // 客户端：通过 Controller SetControlRotation 驱动，角色和摄像机同步转向（无抖动）
       // 服务端：直接 SetActorRotation（DS 无摄像机，不影响）
       // 若未找到目标则不旋转，直接播放 Montage
       {
          AActor* WarpTarget = FindBestWarpTarget();
          if (WarpTarget)
          {
             ACharacter* AvatarChar = GetOwningAvatarCharacter();
             if (AvatarChar)
             {
                // 禁用 CMC 内置旋转，避免与我们的旋转冲突
                if (UCharacterMovementComponent* MoveComp = AvatarChar->GetCharacterMovement())
                {
                   MoveComp->bOrientRotationToMovement = false;
                }

                // 客户端：设 bUseControllerRotationYaw 让角色跟随 Controller
                if (AvatarChar->IsLocallyControlled())
                {
                   bCachedUseControllerRotationYaw = AvatarChar->bUseControllerRotationYaw;
                   AvatarChar->bUseControllerRotationYaw = true;
                }

                const FVector DirToTarget = WarpTarget->GetActorLocation() - AvatarChar->GetActorLocation();
                CachedTargetRotation = DirToTarget.Rotation();
                CachedTargetRotation.Pitch = 0.f;
                CachedTargetRotation.Roll = 0.f;

                GetWorld()->GetTimerManager().SetTimer(
                   RotationTimerHandle,
                   this, &ThisClass::TickRotateToTarget,
                   0.016f, true);  // ~60 fps
             }
          }
       }

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

AActor* UGA_Push_WuKong::FindBestWarpTarget() const
{
    AActor* OwnerAvatar = GetAvatarActorFromActorInfo();
    if (!OwnerAvatar) return nullptr;

    const FVector Origin = OwnerAvatar->GetActorLocation();
    const FVector Forward = OwnerAvatar->GetActorForwardVector();
    const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(WarpConeHalfAngle));

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerAvatar);

    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByChannel(
       Overlaps,
       Origin,
       FQuat::Identity,
       ECC_Pawn,
       FCollisionShape::MakeSphere(WarpTargetDetectionDistance),
       QueryParams);

    AActor* BestTarget = nullptr;
    float BestDot = -1.f;

    for (const FOverlapResult& Overlap : Overlaps)
    {
       AActor* HitActor = Overlap.GetActor();
       if (!HitActor) continue;
       if (!IsActorTeamAttitudeIs(HitActor, ETeamAttitude::Hostile)) continue;
       if (UCAbilitySystemStatics::IsActorDead(HitActor)) continue;

       const FVector DirToTarget = (HitActor->GetActorLocation() - Origin).GetSafeNormal();
       const float Dot = FVector::DotProduct(Forward, DirToTarget);

       // 锥形角度过滤：夹角必须 ≤ WarpConeHalfAngle
       if (Dot < CosHalfAngle) continue;

       // 取前向投影最近的目标（即最接近正前方的目标）
       if (Dot > BestDot)
       {
          BestDot = Dot;
          BestTarget = HitActor;
       }
    }

    return BestTarget;
}

void UGA_Push_WuKong::TickRotateToTarget()
{
    if (!IsActive())
    {
        GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
        return;
    }

    ACharacter* AvatarChar = GetOwningAvatarCharacter();
    if (!AvatarChar)
    {
        GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
        return;
    }

    const float DeltaTime = GetWorld()->GetDeltaSeconds();

    if (AvatarChar->IsLocallyControlled())
    {
        // 客户端：旋转 Controller → 角色自动跟随（bUseControllerRotationYaw=true）
        // 摄像机也跟随 Controller（bUsePawnControlRotation=true）→ 同步转向，无抖动
        if (AController* Controller = AvatarChar->GetController())
        {
            FRotator CurrentRot = Controller->GetControlRotation();
            FRotator TargetRot = CachedTargetRotation;
            TargetRot.Pitch = CurrentRot.Pitch;
            TargetRot.Roll = CurrentRot.Roll;

            const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, WarpRotationInterpSpeed);

            const bool bRotationComplete = FMath::IsNearlyEqual(NewRot.Yaw, CachedTargetRotation.Yaw, 0.5f);
            const FRotator AppliedRot = bRotationComplete
                ? FRotator(CurrentRot.Pitch, CachedTargetRotation.Yaw, CurrentRot.Roll)
                : NewRot;

            Controller->SetControlRotation(AppliedRot);

            if (bRotationComplete)
            {
                GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
            }
        }
    }
    else if (K2_HasAuthority())
    {
        // 服务端（DS）：直接 SetActorRotation，无摄像机
        const FRotator CurrentRot = AvatarChar->GetActorRotation();
        const FRotator NewRot = FMath::RInterpTo(CurrentRot, CachedTargetRotation, DeltaTime, WarpRotationInterpSpeed);

        const bool bRotationComplete = FMath::IsNearlyEqual(NewRot.Yaw, CachedTargetRotation.Yaw, 0.5f);
        const FRotator AppliedRot = bRotationComplete
            ? FRotator(CurrentRot.Pitch, CachedTargetRotation.Yaw, CurrentRot.Roll)
            : NewRot;

        AvatarChar->SetActorRotation(AppliedRot);

        if (bRotationComplete)
        {
            GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
        }
    }
    // 模拟代理：无需处理，服务端SetActorRotation 会通过Actor复制同步
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
    // 停止旋转 Timer（如果还在运行）
    if (RotationTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
    }

    // 移除输入封锁 Tag，恢复移动
    if (ACCharacter* Character = Cast<ACCharacter>(GetOwningAvatarCharacter()))
    {
        if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
        {
            ASC->RemoveLooseGameplayTag(UCAbilitySystemStatics::GetBlockInputStatTag());
        }
        if (Character->IsLocallyControlled())
        {
            Character->bUseControllerRotationYaw = bCachedUseControllerRotationYaw;
        }
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->bOrientRotationToMovement = true;
        }
    }
}

void UGA_Push_WuKong::BlockMoveInput(FGameplayEventData PayLoad)
{
    // 突刺期间通过 Tag 封锁移动输入，不修改 MovementMode
    if (ACCharacter* Character = Cast<ACCharacter>(GetOwningAvatarCharacter()))
    {
        if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
        {
            ASC->AddLooseGameplayTag(UCAbilitySystemStatics::GetBlockInputStatTag());
        }
    }
}

void UGA_Push_WuKong::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                 const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 bool bReplicateEndAbility, bool bWasCancelled)
{
    // 停止旋转 Timer
    if (RotationTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
    }

    // 恢复旋转状态（兜底：如果 EnableMoveInput 未正常触发）
    if (ACCharacter* AvatarChar = Cast<ACCharacter>(GetOwningAvatarCharacter()))
    {
        // 兜底移除输入封锁 Tag
        if (UAbilitySystemComponent* ASC = AvatarChar->GetAbilitySystemComponent())
        {
            ASC->RemoveLooseGameplayTag(UCAbilitySystemStatics::GetBlockInputStatTag());
        }
        if (AvatarChar->IsLocallyControlled())
        {
            AvatarChar->bUseControllerRotationYaw = bCachedUseControllerRotationYaw;
        }
        if (UCharacterMovementComponent* MoveComp = AvatarChar->GetCharacterMovement())
        {
            MoveComp->bOrientRotationToMovement = true;
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}  