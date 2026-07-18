//悟空普攻GA：五段连击，每段Montage包含Attack和 Recovery（恢复姿态）两个Section
//Montage内部有两个等待下次combo区间：
//[recoverystart] ──── 输入窗口 ──── [recoveryend] ──── 恢复尾部 ──── Montage 结束
//Attack期间不接受输入；输入窗口内接受输入；recoveryend之后不再接受输入

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GA_BasicAttack_WuKong.generated.h"

UCLASS()
class CRUNCH_API UGA_BasicAttack_WuKong : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_BasicAttack_WuKong();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// AnimNotify 中配置的 GameplayEvent Tag
	
	static FGameplayTag GetRecoverySectionEventTag();
	static FGameplayTag GetRecoveryEndEventTag();
	static FGameplayTag GetWuKongDamageEventTag();

private:
	// 播放当前段Montage
	void PlayCurrentComboMontage();

	// AnimNotify 触发：进入 Recovery Section（开始接受输入）
	UFUNCTION()
	void OnRecoverySectionEntered(FGameplayEventData EventData);

	// AnimNotify 触发：Recovery 输入窗口结束（不再接受输入）
	UFUNCTION()
	void OnRecoveryEndEntered(FGameplayEventData EventData);

	// Recovery 期间收到输入
	UFUNCTION()
	void OnInputPressDuringRecovery(float TimeWaited);

	// 当前Montage播放完成（Recovery结束且无输入）
	UFUNCTION()
	void OnMontageCompleted();

	// Montage 被打断（可能来自连段过渡或外部因素）
	UFUNCTION()
	void OnMontageInterrupted();

	// 伤害处理
	UFUNCTION()
	void DoDamage(FGameplayEventData Data);

	// 在 Recovery 期间注册 WaitInputPress
	void SetupRecoveryInputWait();

	// 重置段数并结束 GA
	void ResetComboAndEnd();

	// 当前连击段索引（0-4），0 表示第一段
	int32 CurrentComboIndex = 0;

	// 是否已进入 Recovery Section
	bool bIsInRecovery = false;

	// 是否正在过渡到下一段（用于忽略过渡引起的Montage中断回调）
	bool bIsTransitioning = false;

	// 五段连击 Montage，在蓝图子类中配置
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combo")
	TArray<UAnimMontage*> ComboMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> DefaultDamageEffect;
};
