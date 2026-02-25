#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbility.h"
#include "GA_Tornado.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UGA_Tornado : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Montage")
	UAnimMontage* TornadoMontage;

	UFUNCTION()
	void TornadoDamageEventReceived(FGameplayEventData PayLoad);

	UPROPERTY(EditDefaultsOnly, Category="Time")
	float TornadoDuration=5.f;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	float HitPushSpeed = 3000.f ;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TSubclassOf<UGameplayEffect> TornadoDamageEffect;
};
