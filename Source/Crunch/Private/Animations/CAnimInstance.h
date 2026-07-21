#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CAnimInstance.generated.h"

struct FGameplayTag;
/******************** Locomotion State Machine Transitions ********************/
// 四状态机：Ground ⇄ Falling ⇄ Jump ⇄ JumpRecovery
//
// Ground ──→ Jump:          MovementMode变为Falling 且 JumpCount>0（主动起跳）
// Ground ──→ Falling:       MovementMode变为Falling 且 JumpCount==0（被动腾空）
//
// Falling ─→ Jump:          腾空期间JumpCount从0变为>0（空中起跳）
// Falling ─→ JumpRecovery:  落地（MovementMode==Walking）
//
// Jump ───→ JumpRecovery:   落地（MovementMode==Walking）
//
// JumpRecovery ─→ Jump:     落地恢复期间起跳，打断恢复（优先级高于→Ground）
// JumpRecovery ─→ Falling:  落地恢复期间被击飞/推落，打断恢复（优先级高于→Ground）
// JumpRecovery ─→ Ground:   在地面持续时间 >= JumpRecoveryDuration（未被任何打断）
UCLASS()
class UCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	//相当于AnimInst的构造函数，用于为OwnerMovementComp赋值
	virtual void NativeInitializeAnimation() override;

	//从GameThread中获得需要的数据并缓存，作为读取World的接口
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	//利用缓存好的数据计算动画状态，从而得出AnimGraph需要的逻辑结果
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	
	//获取角色当前速度
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetSpeed() const {return Speed;}

	//是否正在移动(速度不为0)
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsMoving() const {return Speed!=0;}

	//是否没有移动（速度为0）
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsNotMoving() const {return Speed==0;}
	
	//获取经平滑处理的水平转向速度
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetSmoothYawSpeed() const {return SmoothYawSpeed; }
	
	// 直接暴露缓存的 MovementMode，AnimBP 中可与 MOVE_Falling 做精确比较
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE EMovementMode GetMovementMode() const { return MovementMode; }

	// 便捷条件：MovementMode == MOVE_Falling（替代 IsFalling 用于状态机过渡）
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsMovementModeFalling() const { return MovementMode == MOVE_Falling; }

	// 当前跳跃段数：0=地面, 1=一段跳, 2=二段跳（从 Character->JumpCurrentCount 每一帧缓存）
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE int32 GetJumpCount() const {return JumpCount;}

	// 是否处于AimStat
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsAiming() const {return bIsAiming;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsOnGround() const {return MovementMode == MOVE_Walking;}
	
	//获得前向速度
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetFwdSpeed() const {return FwdSpeed;}

	//获得径向速度(取向右为正方向)
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetRightSpeed() const {return RightSpeed;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE bool HasAcceleration() const {return Acceleration > 0.f;}


	// AnimBP 过渡条件：标记一二段跳，区分"主动跳跃"与"被动腾空"（击飞/坠落等）
	// JumpCurrentCount 在 Jump() 时自增、Landed() 时清零，自然标记跳跃段数
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe))
	bool IsInFirstJump() const { return JumpCount == 1; }

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe))
	bool IsInSecondJump() const { return JumpCount == 2; }
	
	// Ground → Jump: 在地面时主动按下跳跃（MovementMode变为Falling且JumpCount>0）
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe))
	bool ShouldEnterJump() const { return IsMovementModeFalling() && JumpCount > 0; }

	// Ground → Falling: 在地面时被动腾空（MovementMode变为Falling但JumpCount==0）
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe))
	bool ShouldEnterFalling() const { return IsMovementModeFalling() && JumpCount == 0; }

	// Falling → Jump: 腾空期间触发了跳跃（JumpCount从0变为>0，即空中一段跳/二段跳）
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe))
	bool ShouldAirJump() const { return JumpCount > PrevJumpCount && JumpCount > 0; }

	// Jump / Falling → JumpRecovery: 触地瞬间（MovementMode回到Walking）
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe))
	bool ShouldEnterJumpRecovery() const { return GetIsOnGround(); }

	// JumpRecovery → Ground: 落地恢复完成（在地面停留超过JumpRecoveryDuration）
	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe))
	bool IsJumpRecoveryComplete() const { return  GetTimeOnGround() >= JumpRecoveryDuration; }

	// 当前在地面上连续停留的时间（秒），用于驱动 JumpRecovery → Ground 过渡
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetTimeOnGround() const { return TimeOnGround; }


	//锁定的停步方向：0=Forward, 1=Backward, 2=Left, 3=Right
	//在HasAcceleration为true且高速移动时持续更新，松手瞬间锁住
	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE int32 GetLockedStopDir() const {return LockedStopDir;}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetLookYawOffset() const {return FMath::UnwindDegrees(LookRotOffset.Yaw);}

	UFUNCTION(BlueprintCallable,meta=(BlueprintThreadSafe))
	FORCEINLINE float GetLookPitchOffset() const {return LookRotOffset.Pitch;}

	//静止并且非瞄准下才使用UpperBody当做FullBody，其他情况还是使用UpperBody与Locomotion的混合，使得Aim下即使静止下半身也是跟随转向运动的
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

	float Acceleration;

	bool bIsJumping;
	
	//锁定的停步方向，仅在高速移动时更新，松手瞬间锁住
	//0=Forward, 1=Backward, 2=Left, 3=Right
	int32 LockedStopDir = 0;

	//只有速度超过此阈值时才会锁定停步方向，低于此值直接回Idle不播停步动画
	UPROPERTY(EditAnywhere, Category = "Animation")
	float StopSpeedThreshold = 200.f;


	// 缓存的 MovementMode（从 CharacterMovementComponent 每帧读取）
	EMovementMode MovementMode = MOVE_None;

	// 跳跃段数：0=地面, 1=一段跳, 2=二段跳

	int32 JumpCount = 0;
	bool bIsAiming;

	// 上一帧的跳跃段数，用于检测"空中起跳"（Falling → Jump 过渡）
	int32 PrevJumpCount = 0;

	// 在地面上连续停留的时间（秒），腾空时清零
	// 用于驱动 JumpRecovery → Ground 过渡
	float TimeOnGround = 0.f;

	// JumpRecovery 状态最短持续时间，到达后自动过渡到 Ground
	UPROPERTY(EditAnywhere, Category="Animation|Locomotion")
	float JumpRecoveryDuration = 0.3f;

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
