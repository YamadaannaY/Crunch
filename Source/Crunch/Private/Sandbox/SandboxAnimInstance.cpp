#include "SandboxAnimInstance.h"
#include "Sandbox/SandboxCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void USandboxAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ACharacter* OwnerChar = Cast<ACharacter>(TryGetPawnOwner());
	if (!OwnerChar) return;

	UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement();
	if (!MoveComp) return;

	// ──── Stride ──────────────────────────────────────────────────
	const float MaxSpeed = MoveComp->MaxWalkSpeed * StrideMaxSpeedScale;
	
	bHasMovementInput = MoveComp->GetCurrentAcceleration().SizeSquared() > 0.0f;

	Stride = (MaxSpeed > 0.f) ? FMath::Clamp(GetSpeed() / MaxSpeed, 0.f, 1.f) : 0.f;

	// ──── WalkRun ─────────────────────────────────────────────────
	float TargetWalkRun = 1.f;
	if (ASandboxCharacter* SandboxChar = Cast<ASandboxCharacter>(OwnerChar))
	{
		bWalkMode = SandboxChar->IsWalkMode();
		TargetWalkRun = SandboxChar->IsWalkMode() ? 0.f : 1.f;
	}
	WalkRun = FMath::FInterpTo(WalkRun, TargetWalkRun, DeltaSeconds, WalkRunInterpSpeed);

	// ──── Stand Idle State Machine ────────────────────────────────
	//
	//   常驻 Idle ──Timer 到──▶ Stand_Action ──播完──▶ 常驻 Idle
	//     Index=0               Index=1..N               Index=0
	//
	//   移动或离地 → 强制回到常驻 Idle + 清零所有状态
	//
	const bool bIsStandingStill = GetSpeed() <= 0.f && GetIsOnGround();

	if (!bIsStandingStill)
	{
		StandIdleIndex = 0;
		StandIdleTimer = 0.f;
		StandActionTimer = 0.f;
		CurrentActionDuration = 0.f;
		StandIdlePrevActionIndex = INDEX_NONE;
		return;
	}

	// ── 常驻 Idle 阶段 ───────────────────────────────────────────
	if (StandIdleIndex == 0)
	{
		StandIdleTimer += DeltaSeconds;

		if (StandIdleTimer >= StandIdleChangeInterval)
		{
			StandIdleTimer = 0.f;

			// 随机选一个 Action（≠ 上一轮）
			do
			{
				StandIdleIndex = FMath::RandRange(1, StandActionVariantCount);
			} while (StandIdleIndex == StandIdlePrevActionIndex && StandActionVariantCount > 1);

			StandIdlePrevActionIndex = StandIdleIndex;
			StandActionTimer = 0.f;

			// 从数组读取该 Action 的时长；未覆盖则回落默认值
			const int32 DurationIndex = StandIdleIndex - 1; // Action[1]=数组[0]
			if (StandActionDurations.IsValidIndex(DurationIndex))
			{
				CurrentActionDuration = StandActionDurations[DurationIndex];
			}
			else
			{
				CurrentActionDuration = DefaultActionDuration;
			}
		}
	}
	// ── Stand_Action 阶段 ────────────────────────────────────────
	else
	{
		StandActionTimer += DeltaSeconds;

		if (StandActionTimer >= CurrentActionDuration)
		{
			// 播完 → 回到常驻 Idle，开始重新计时
			StandIdleIndex = 0;
			StandIdleTimer = 0.f;
			StandActionTimer = 0.f;
			// StandIdlePrevActionIndex 保留，防止下一轮选到同一个
		}
	}
}

void USandboxAnimInstance::OnFootPlantNotify(EFootPlant Foot)
{
	Foot == EFootPlant::Left ? EnterLeftStop = true : EnterLeftStop = false;
}
