// 悟空普攻：五段连击，每段Montage包含 Attack（攻击）和 Recovery（恢复姿态）两个Section
// Recovery 内部有两个子区间：
//   [recovery] ──── 输入窗口 ──── [recoveryend] ──── 恢复尾部 ──── Montage 结束
//   Attack期间不接受输入；输入窗口内接受输入；recoveryend之后不再接受输入
// 输入窗口内有输入 → 进入下一段（最后一段则跳回第一段循环）；Recovery完成后无输入则重置段数

#include "GA_BasicAttack_WuKong.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Character/CCharacter.h"

UGA_BasicAttack_WuKong::UGA_BasicAttack_WuKong()
{
	// 防止自身重复激活
	AbilityTags.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_BasicAttack_WuKong::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		// 监听 Recovery 输入窗口开始（持续监听，贯穿整个 GA 生命周期）
		UAbilityTask_WaitGameplayEvent* WaitRecoveryTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, GetRecoverySectionEventTag(), nullptr, false, false);
		WaitRecoveryTask->EventReceived.AddDynamic(this, &ThisClass::OnRecoverySectionEntered);
		WaitRecoveryTask->ReadyForActivation();

		// 监听 Recovery 输入窗口结束
		UAbilityTask_WaitGameplayEvent* WaitRecoveryEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, GetRecoveryEndEventTag(), nullptr, false, false);
		WaitRecoveryEndTask->EventReceived.AddDynamic(this, &ThisClass::OnRecoveryEndEntered);
		WaitRecoveryEndTask->ReadyForActivation();

		// 播放第一段 Montage
		CurrentComboIndex = 0;
		PlayCurrentComboMontage();
	}

	// 服务端监听伤害事件
	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitDamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, GetWuKongDamageEventTag());
		WaitDamageTask->EventReceived.AddDynamic(this, &ThisClass::DoDamage);
		WaitDamageTask->ReadyForActivation();
	}
}

FGameplayTag UGA_BasicAttack_WuKong::GetRecoverySectionEventTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("ability.combo.sectionstart"));
}

FGameplayTag UGA_BasicAttack_WuKong::GetRecoveryEndEventTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("ability.combo.sectionend"));
}

FGameplayTag UGA_BasicAttack_WuKong::GetWuKongDamageEventTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("ability.combo.damage"));
}

void UGA_BasicAttack_WuKong::PlayCurrentComboMontage()
{
	if (CurrentComboIndex >= ComboMontages.Num())
	{
		ResetComboAndEnd();
		return;
	}

	UAnimMontage* MontageToPlay = ComboMontages[CurrentComboIndex];
	if (!MontageToPlay)
	{
		ResetComboAndEnd();
		return;
	}

	// 重置Recovery状态
	bIsInRecovery = false;
	
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay);
	PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	PlayMontageTask->ReadyForActivation();

	// 过渡标记置 false，此后该 Montage 的中断才会被处理
	bIsTransitioning = false;
}

void UGA_BasicAttack_WuKong::OnRecoverySectionEntered(FGameplayEventData EventData)
{
	if (bIsTransitioning) return;

	bIsInRecovery = true;
	SetupRecoveryInputWait();
}

void UGA_BasicAttack_WuKong::OnRecoveryEndEntered(FGameplayEventData EventData)
{
	if (bIsTransitioning) return;

	bIsInRecovery = false;
}

void UGA_BasicAttack_WuKong::SetupRecoveryInputWait()
{
	UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this, &ThisClass::OnInputPressDuringRecovery);
	WaitInputPress->ReadyForActivation();
}

void UGA_BasicAttack_WuKong::OnInputPressDuringRecovery(float TimeWaited)
{
	// 只在 Recovery 输入窗口期间且未在过渡中才响应
	if (!bIsInRecovery || bIsTransitioning) return;

	// 二段跳期间禁止连段
	if (const ACCharacter* Char = Cast<ACCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (Char->JumpCurrentCount >= 2) return;
	}

	bIsTransitioning = true;
	bIsInRecovery = false;
	CurrentComboIndex++;

	if (CurrentComboIndex >= ComboMontages.Num())
	{
		// 最后一段的输入窗口内收到输入 → 跳回第一段，循环连击
		CurrentComboIndex = 0;
	}

	// 播放下一段 Montage（内部会创建新的 PlayMontageAndWait 任务，
	// 旧的 Montage 被自动停止并触发 OnMontageInterrupted，那时 bIsTransitioning=true 所以会被忽略）
	PlayCurrentComboMontage();
}

void UGA_BasicAttack_WuKong::OnMontageCompleted()
{
	// Montage 自然播放完成 = Recovery 尾部结束且无输入 → 重置段数
	if (bIsTransitioning) return;
	
	ResetComboAndEnd();
}

void UGA_BasicAttack_WuKong::OnMontageInterrupted()
{
	// 被打断时，如果正在过渡中则忽略，否则重置
	if (bIsTransitioning) return;
	
	ResetComboAndEnd();
}

void UGA_BasicAttack_WuKong::DoDamage(FGameplayEventData Data)
{
	const int HitCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(Data.TargetData);

	for (int i = 0; i < HitCount; i++)
	{
		FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Data.TargetData, i);
		if (HitResult.GetActor())
		{
			ApplyGameplayEffectToHitResultActor(HitResult, DefaultDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}

void UGA_BasicAttack_WuKong::ResetComboAndEnd()
{
	CurrentComboIndex = 0;
	bIsInRecovery = false;
	bIsTransitioning = false;
	K2_EndAbility();
}
