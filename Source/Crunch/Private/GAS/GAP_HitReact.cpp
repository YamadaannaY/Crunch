#include "GAP_HitReact.h"
#include "AbilitySystemComponent.h"
#include "Character/CCharacter.h"
#include "GameplayTagContainer.h"
#include "Crunch/DebugHelper.h"
#include "UCAbilitySystemStatics.h"

UGAP_HitReact::UGAP_HitReact()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

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

	if (!TriggerEventData)
	{
		K2_EndAbility();
		return;
	}

	const AActor* Attacker = TriggerEventData->ContextHandle.GetInstigator();
	if (!Attacker)
	{
		Debug::Print(TEXT("[HitReact] No Attacker"), FColor::Red);
		K2_EndAbility();
		return;
	}

	ACCharacter* VictimChar = Cast<ACCharacter>(GetAvatarActorFromActorInfo());
	if (!VictimChar)
	{
		K2_EndAbility();
		return;
	}

	float OutDist;
	const FGameplayTag DirectionTag = ComputeHitReactDirectionTag(Attacker, VictimChar, OutDist);

	// 服务端播放 + NetMulticast RPC 到所有客户端
	VictimChar->ServerPlayHitReactMontage(DirectionTag);

	K2_EndAbility();
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

	if (OutAngleDifference >= -45.f && OutAngleDifference <= 45.f)  return GetHitReactFrontTag();
	if (OutAngleDifference > 45.f && OutAngleDifference <= 135.f)   return GetHitReactRightTag();
	if (OutAngleDifference > 135.f || OutAngleDifference <= -135.f) return GetHitReactBackTag();
	if (OutAngleDifference < -45.f && OutAngleDifference > -135.f)  return GetHitReactLeftTag();

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
