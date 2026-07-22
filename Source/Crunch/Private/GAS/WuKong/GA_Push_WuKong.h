// 悟空突刺：ActivateAbility 阶段先做前向锥形检测（±WarpConeHalfAngle°），
// 若检测到敌对目标则在 Montage 播放期间通过 RInterpTo 平滑旋转角色朝向目标；
// 若未检测到目标则不旋转直接释放。
// AnimNotify 触发突刺检测阶段：前向 SphereSweep 一次性捕获突刺路径上的所有敌对目标，
// 应用 FGenericDamageEffectDef：DamageEffect = 伤害 GE；PushVelocity = 击退速度（X=前向，Z=竖直）
// 伤害检测仅在服务端进行（Server-Authoritative）
// GA 期间禁止移动输入和跳跃输入，EndAbility 时恢复

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbilitiesType.h"
#include "GAS/CGameplayAbility.h"
#include "GA_Push_WuKong.generated.h"

UCLASS()
class CRUNCH_API UGA_Push_WuKong : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Push_WuKong();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	static FGameplayTag GetPushAbilityTag() { return FGameplayTag::RequestGameplayTag(TEXT("ability.push.wukong")); }

private:
	//前向 SphereSweep 检测突刺路径上的目标并施加伤害+击退
	UFUNCTION()
	void StartPushPhase();

	// 锥形检测：在前向 ±WarpConeHalfAngle° 范围内搜索最近的敌对目标
	AActor* FindBestWarpTarget() const;

	// 每帧 RInterpTo 平滑旋转朝向目标，由 RotationTimerHandle 驱动
	void TickRotateToTarget();

	// 突刺 Montage（武器延展向前刺击）
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* PushMontage;

	// SphereSweep 半径
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float SweepRadius = 80.f;

	// 前向 Sweep 距离（覆盖武器最大延伸范围）
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float ThrustSweepDistance = 500.f;

	// 伤害 GE + 击退速度（PushVelocity：X=前向水平速度，Z=竖直速度）
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	FGenericDamageEffectDef PushDamageEffectDef;

	// 等待 AnimNotify 触发突刺的 Tag
	static FGameplayTag GetBeginPushTag() { return FGameplayTag::RequestGameplayTag(TEXT("ability.push.wukong.begin")); }

	// 目标搜索距离
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Rotation")
	float WarpTargetDetectionDistance = 600.f;

	// 锥形检测半角（度），总锥角 = 2 × 此值（默认 20° → 共 40°）
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Rotation")
	float WarpConeHalfAngle = 20.f;

	// RInterpTo 旋转插值速度，值越大转向越快（建议 8~15）
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Rotation")
	float WarpRotationInterpSpeed = 12.f;

	// 旋转定时器句柄
	FTimerHandle RotationTimerHandle;

	// 缓存的目标朝向（Yaw-only），在 ActivateAbility 时计算
	FRotator CachedTargetRotation;

	// 缓存旋转前的 bUseControllerRotationYaw，EndAbility / EnableMoveInput 时恢复
	bool bCachedUseControllerRotationYaw = false;

	UFUNCTION()
	void  EnableMoveInput(FGameplayEventData PayLoad);
	UFUNCTION()
	void  BlockMoveInput(FGameplayEventData PayLoad);
};
