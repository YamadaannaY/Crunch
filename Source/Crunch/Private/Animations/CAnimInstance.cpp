#include "Animations/CAnimInstance.h"
#include  "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Kismet/KismetMathLibrary.h"

void UCAnimInstance::NativeInitializeAnimation()
{

	OwnerCharacter=Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComp=OwnerCharacter->GetCharacterMovement();
	}

	UAbilitySystemComponent* OwnerASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
	if (OwnerASC)
	{
		OwnerASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetAimStatTag()).AddUObject(this,&ThisClass::OwnerAimTagChanged);
	}
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (OwnerCharacter)
	{
		FVector Vel=OwnerCharacter->GetVelocity();
		Speed=Vel.Length();

		const FRotator BodyRot=OwnerCharacter->GetActorRotation();

		//获得Rotator增量并归一化，归一化是为了匹配BS
		const FRotator BodyRotDelta=UKismetMathLibrary::NormalizedDeltaRotator(BodyRot,BodyPrevRot);

		//更新PrevRot
		BodyPrevRot=BodyRot;

		//获得旋转速度
		YawSpeed=BodyRotDelta.Yaw/DeltaSeconds;

		float YawLerpSpeed=YawSpeedSmoothLerpSpeed;

		if (YawSpeed == 0 )
		{
			YawLerpSpeed=YawSpeedLerpToZeroSpeed;
		}

		SmoothYawSpeed=UKismetMathLibrary::FInterpTo(SmoothYawSpeed,YawSpeed,DeltaSeconds,YawLerpSpeed);

		//BaseAimRotation：Character的实际视野朝向，包括了Pitch
		FRotator ControlRot=OwnerCharacter->GetBaseAimRotation();

		//获得视线和身体角度的差值，即BS中头部要转过的角度。
		LookRotOffset=UKismetMathLibrary::NormalizedDeltaRotator(ControlRot,BodyRot);

		//将速度向量进行点乘投影，叉乘计算，最后获得在合速度在视角朝向方向的前向速度和右向速度
		FwdSpeed=Vel.Dot(ControlRot.Vector());
		RightSpeed=-Vel.Dot(ControlRot.Vector().Cross(FVector::UpVector));
	}

	if (OwnerMovementComp)
	{
		//角色当前不在地面上时Falling为True，原理是每一帧进行向下sweep检测是否有可以站立的地面
		bIsJumping=OwnerMovementComp->IsFalling();
		Acceleration = OwnerMovementComp->GetCurrentAcceleration().Length();
	}

	if (OwnerCharacter)
	{
		// 缓存跳跃段数：0=地面, 1=一段跳, 2=二段跳
		JumpCount = OwnerCharacter->JumpCurrentCount;
	}

	//锁定停步方向：有加速度输入 且 高速移动时 持续更新；松手瞬间锁住
	if (HasAcceleration() && Speed > StopSpeedThreshold)
	{
		const float AbsFwd = FMath::Abs(FwdSpeed);
		const float AbsRight = FMath::Abs(RightSpeed);

		if (AbsFwd >= AbsRight)
		{
			LockedStopDir = (FwdSpeed > 0.f) ? 0 : 1;
		}
		else
		{
			LockedStopDir = (RightSpeed > 0.f) ? 2 : 3;
		}
	}
}

void UCAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}

bool UCAnimInstance::bShouldDoFullBody() const
{
	return GetSpeed()<=0 && !GetIsAiming();
}

void UCAnimInstance::OwnerAimTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	bIsAiming= NewCount!=0;
}
