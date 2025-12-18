// Fill out your copyright notice in the Description page of Project Settings.


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

	UAbilitySystemComponent* OwnerASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TryGetPawnOwner());
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
		
		FRotator BodyRot=OwnerCharacter->GetActorRotation();
		
		//获得Rotator增量并归一化，归一化是为了匹配BS
		const FRotator BodyRotDelta=UKismetMathLibrary::NormalizedDeltaRotator(BodyRot,BodyPrevRot);

		//更新PrevRot
		BodyPrevRot=BodyRot;
		//通过差值与时间的差值获得旋转速度
		YawSpeed=BodyRotDelta.Yaw/DeltaSeconds;
		
		SmoothYawSpeed=UKismetMathLibrary::FInterpTo(SmoothYawSpeed,YawSpeed,DeltaSeconds,YawSpeedSmoothLerpSpeed);

		//BaseAimRotation是Controller的实际朝向
		FRotator ControlRot=OwnerCharacter->GetBaseAimRotation();
		//获得差值，这个差值就是BS中要转过的角度。
		LookRotOffset=UKismetMathLibrary::NormalizedDeltaRotator(ControlRot,BodyRot);

		FwdSpeed=Vel.Dot(ControlRot.Vector());
		RightSpeed=-Vel.Dot(ControlRot.Vector().Cross(FVector::UpVector));
		
	}

	if (OwnerMovementComp)
	{
		//角色当前不在地面上时Falling为True，原理是每一帧进行向下sweep检测是否有可以站立的地面
		bIsJumping=OwnerMovementComp->IsFalling();
	}
}

void UCAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}

bool UCAnimInstance::bShouldDoFullBoday() const
{
	return (GetSpeed() <=0 && !(GetIsAiming()));
}

void UCAnimInstance::OwnerAimTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	bIsAiming= NewCount!=0;
}
