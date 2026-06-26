// 被动GA，ServerOnly：服务端计算方向 → 调 ACCharacter::ServerPlayHitReactMontage
// Character 内部服务器播放 + NetMulticast RPC → 所有客户端本地播放

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GAP_HitReact.generated.h"

UCLASS()
class CRUNCH_API UGAP_HitReact : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGAP_HitReact();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	static FGameplayTag GetHitReactEventTag();
	static FGameplayTag GetHitReactFrontTag();
	static FGameplayTag GetHitReactBackTag();
	static FGameplayTag GetHitReactLeftTag();
	static FGameplayTag GetHitReactRightTag();

private:
	FGameplayTag ComputeHitReactDirectionTag(const AActor* InAttacker, AActor* InVictim, float& OutAngleDifference);
};
