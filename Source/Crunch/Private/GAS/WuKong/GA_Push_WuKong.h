// 悟空突刺：播放突刺 Montage（武器延展向前刺击），通过 AnimNotify 触发突刺检测阶段，
// 在触发时做前向 SphereSweep 一次性捕获突刺路径上的所有敌对目标，
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

	// 突刺前的 MovementMode，EndAbility 时恢复
	EMovementMode CachedMovementMode = MOVE_Walking;
	
	UFUNCTION()
	void  EnableMoveInput(FGameplayEventData PayLoad);
	UFUNCTION()
	void  BlockMoveInput(FGameplayEventData PayLoad);
};
