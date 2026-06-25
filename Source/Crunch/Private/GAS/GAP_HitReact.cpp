#include "GAP_HitReact.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Crunch/DebugHelper.h"
#include "UCAbilitySystemStatics.h"

UGAP_HitReact::UGAP_HitReact()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = GetHitReactEventTag();
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	ActivationBlockedTags.RemoveTag(UCAbilitySystemStatics::GetStunStatTag());
}

void UGAP_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                    const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	Debug::Print(TEXT("UGAP_HitReact::ActivateAbility"));

	// 清除上次可能残留的 Timer 和 Tag（快速连续受击场景）
	if (TagRemovalTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TagRemovalTimerHandle);
	}
	if (AppliedDirectionTag.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveReplicatedLooseGameplayTag(AppliedDirectionTag);
		}
		AppliedDirectionTag = FGameplayTag::EmptyTag;
	}

	if (!TriggerEventData)
	{
		K2_EndAbility();
		return;
	}

	// 获取攻击者
	AActor* Attacker = TriggerEventData->ContextHandle.GetInstigator();
	if (!Attacker)
	{
		Debug::Print(TEXT("HitReact: No Attacker in ContextHandle"), FColor::Red);
		K2_EndAbility();
		return;
	}

	AActor* Victim = GetAvatarActorFromActorInfo();
	if (!Victim)
	{
		K2_EndAbility();
		return;
	}
	Debug::Print(FString::Printf(TEXT("HitReact Activated! Victim: %s"),
			 *GetNameSafe(Victim)), FColor::Red);
	// 计算受击方向
	float OutAngleDifference;
	const FGameplayTag DirectionTag = ComputeHitReactDirectionTag(Attacker, Victim, OutAngleDifference);

	// ServerOnly：添加 ReplicatedLooseGameplayTag，自动复制到所有客户端
	// 客户端收到后由 ACCharacter::HitReactDirectionTagUpdated 本地播放 Montage
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->AddReplicatedLooseGameplayTag(DirectionTag);
		AppliedDirectionTag = DirectionTag;
		Debug::Print(FString::Printf(TEXT("[HitReact] Added Tag: %s -> %s"), *DirectionTag.ToString(), *GetNameSafe(Victim)), FColor::Orange);

		// 定时移除 Tag（时长匹配受击动画）
		GetWorld()->GetTimerManager().SetTimer(TagRemovalTimerHandle,
			FTimerDelegate::CreateUObject(this, &UGAP_HitReact::OnTagRemovalTimer),
			HitReactTagDuration, false);
	}

	K2_EndAbility();
}

void UGAP_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
                               const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo,
                               bool bReplicateEndAbility, bool bWasCancelled)
{
	// 能力被取消时清除 Timer 和 Tag
	if (bWasCancelled && TagRemovalTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TagRemovalTimerHandle);
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveReplicatedLooseGameplayTag(AppliedDirectionTag);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGAP_HitReact::OnTagRemovalTimer()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveReplicatedLooseGameplayTag(AppliedDirectionTag);
	}
	AppliedDirectionTag = FGameplayTag::EmptyTag;
	TagRemovalTimerHandle.Invalidate();
}

FGameplayTag UGAP_HitReact::ComputeHitReactDirectionTag(const AActor* InAttacker, AActor* InVictim,
                                                        float& OutAngleDifference)
{
	check(InAttacker && InVictim);

	const FVector VictimForward = InVictim->GetActorForwardVector();
	const FVector VictimToAttackerNormalized = (InAttacker->GetActorLocation() - InVictim->GetActorLocation()).GetSafeNormal();

	const float DotResult = FVector::DotProduct(VictimForward, VictimToAttackerNormalized);
	OutAngleDifference = FMath::RadiansToDegrees(FMath::Acos(DotResult));

	const FVector CrossResult = FVector::CrossProduct(VictimForward, VictimToAttackerNormalized);
	if (CrossResult.Z < 0.f)
	{
		OutAngleDifference *= -1.f;
	}

	if (OutAngleDifference >= -45.f && OutAngleDifference <= 45.f)
	{
		return GetHitReactFrontTag();
	}
	if (OutAngleDifference > 45.f && OutAngleDifference <= 135.f)
	{
		return GetHitReactRightTag();
	}
	if (OutAngleDifference > 135.f || OutAngleDifference <= -135.f)
	{
		return GetHitReactBackTag();
	}
	if (OutAngleDifference < -45.f && OutAngleDifference > -135.f)
	{
		return GetHitReactLeftTag();
	}

	return GetHitReactFrontTag();
}

FGameplayTag UGAP_HitReact::GetHitReactEventTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("ability.passive.hitreact.activate"));
}

FGameplayTag UGAP_HitReact::GetHitReactFrontTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("stats.hitreact.front"));
}

FGameplayTag UGAP_HitReact::GetHitReactBackTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("stats.hitreact.back"));
}

FGameplayTag UGAP_HitReact::GetHitReactLeftTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("stats.hitreact.left"));
}

FGameplayTag UGAP_HitReact::GetHitReactRightTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("stats.hitreact.right"));
}
