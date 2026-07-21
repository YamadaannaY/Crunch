// 悟空翻滚：Launch 起跳 → RollMontage（翻滚+滞空，Section 循环）→ 落地结束
// 全程不干预重力，依靠 Launch 初速度形成自然抛物线弧
// AnimBP 的 SequencePlayer 关闭 Root Motion，位移完全由 GA 代码驱动

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GA_Roll_WuKong.generated.h"

UCLASS()
class CRUNCH_API UGA_Roll_WuKong : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Roll_WuKong();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	static FGameplayTag GetRollAbilityTag() { return FGameplayTag::RequestGameplayTag(TEXT("ability.roll.wukong")); }

private:
	void StartRollPhase();

	// 翻滚阶段每帧前冲
	void TickRollForward();
	FVector GetRollDirection() const;

	UFUNCTION()
	void OnRollMontageCompleted();
	UFUNCTION()
	void OnRollMontageInterrupted();
	
	// 轮询落地（替代 LandedDelegate，客户端/服务端各自独立检测）
	void TickCheckLanding();

	// 翻滚 Montage（翻滚 + 滞空循环 Section）
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RollMontage;

	// Launch 前冲速度（cm/s）
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float InitialBurstSpeed = 600.f;

	// Launch 向上速度（cm/s）
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float InitialUpwardSpeed = 1000.f;

	// 翻滚阶段每帧前冲速度（cm/s）
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RollSpeed = 600.f;

	// 是否允许输入转向
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	bool bAllowSteering = true;

	FTimerHandle RollTimerHandle;
};
