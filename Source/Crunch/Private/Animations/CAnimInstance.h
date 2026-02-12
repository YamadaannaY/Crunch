// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CAnimInstance.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class UCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	//相当于构造函数，用于为OwnerMovementComp赋值
	virtual void NativeInitializeAnimation() override;

	//从GameThread中获得需要的数据并缓存，作为读取World的接口
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	//利用缓存好的数据计算动画状态，从而得出AnimGraph需要的逻辑结果
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	/****************	ABP中使用的接口函数 *******************/
	
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetSpeed() const {return Speed;}
	
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsMoving() const {return Speed!=0;}
	
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsNotMoving() const {return Speed==0;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetYawSpeed() const {return YawSpeed ;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetSmoothYawSpeed() const {return SmoothYawSpeed; }

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsJumping() const {return bIsJumping;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsAiming() const {return bIsAiming;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsOnGround() const {return !bIsJumping;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetFwdSpeed() const {return FwdSpeed;}
	
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetRightSpeed() const {return RightSpeed;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetLookYawOffset() const {return LookRotOffset.Yaw;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetLookPitchOffset() const {return LookRotOffset.Pitch;}

	//在静止非瞄准下才使用FullBody，如果静止但是处于AimStat，还是使用UpperBody（没有过滤下半部分的Anim，使角色在原地移动）
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	bool bShouldDoFullBody() const ;

private:
	void OwnerAimTagChanged(const FGameplayTag Tag,int32 NewCount);
	
	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY()
	class UCharacterMovementComponent*  OwnerMovementComp;

	float Speed;

	float YawSpeed;

	float SmoothYawSpeed;

	float FwdSpeed;
	
	float RightSpeed;

	bool bIsJumping;
	
	bool bIsAiming;

	//由于鼠标控制旋转，速度和变化都非常快，所以手动设置一个LerpSeed，控制插值速度，使用InterpTo进行在这个速度下平滑旋转
	UPROPERTY(EditAnywhere,Category="Animation")
	float YawSpeedSmoothLerpSpeed=1.f;

	//当YawSpeed为0时，BS从其他节点返回默认状态，这个过程需要快一点，不然角色即使静止也会因为回到默认状态太慢而依旧出现晃动
	UPROPERTY(EditAnywhere,Category="Animation")
	float YawSpeedLerpToZeroSpeed=30.f;

	//记录上一个Rotator
	FRotator BodyPrevRot;

	//获得头部旋转的角度
	FRotator LookRotOffset;
};
