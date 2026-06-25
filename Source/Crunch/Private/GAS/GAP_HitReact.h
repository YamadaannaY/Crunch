// 被动GA，ServerOnly，与 GAP_Dead 统一模式：
// 服务端计算受击方向 → 添加对应的 ReplicatedLooseGameplayTag（自动复制到客户端）
// → ACCharacter 的 RegisterGameplayTagEvent 回调 → 本地 PlayAnimMontage

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GAP_HitReact.generated.h"

/*
 * 被动技能：受击反应
 * 不直接播放 Montage。计算受击方向后，添加对应的 GameplayTag，
 * Tag 复制到客户端后由 ACCharacter::HitReactDirectionTagUpdated 本地播放动画。
 */
UCLASS()
class CRUNCH_API UGAP_HitReact : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGAP_HitReact();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 用于触发此被动GA的 EventTag
	static FGameplayTag GetHitReactEventTag();

	// 受击方向 Tag（GA 添加，Character 监听）
	static FGameplayTag GetHitReactFrontTag();
	static FGameplayTag GetHitReactBackTag();
	static FGameplayTag GetHitReactLeftTag();
	static FGameplayTag GetHitReactRightTag();

private:
	// 根据攻击者和受害者的世界朝向计算受击方向 Tag
	FGameplayTag ComputeHitReactDirectionTag(const AActor* InAttacker, AActor* InVictim, float& OutAngleDifference);

	// Tag 持续时间（到期后自动 RemoveReplicatedLooseGameplayTag，匹配受击动画长度）
	UPROPERTY(EditDefaultsOnly, Category = "HitReact")
	float HitReactTagDuration = 0.8f;

	// 用于到期后移除 Tag 的 Timer
	FTimerHandle TagRemovalTimerHandle;

	// Tag 移除 Timer 回调
	void OnTagRemovalTimer();

	// 当前应用的方向 Tag（Timer 回调时移除用）
	FGameplayTag AppliedDirectionTag;
};
