// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include  "UCAbilitySystemStatics.generated.h"

struct FGameplayAbilitySpec;
class UGameplayAbility;
/**
 * 
 */
UCLASS()
class UCAbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static FGameplayTag GetBasicAttackAbilityTag();
	static FGameplayTag GetBasicAttackInputPressedTag();
	static FGameplayTag GetDeadStatTag();
	static FGameplayTag GetStunStatTag();
	static FGameplayTag GetAimStatTag();
	static FGameplayTag GetHealthFullStatTag();
	static FGameplayTag GetHealthEmptyStatTag();
	static FGameplayTag GetManaFullStatTag();
	static FGameplayTag GetManaEmptyStatTag();
	static FGameplayTag GetCameraShakeCueTag();
	static FGameplayTag GetGoldAttributeTag();
	static FGameplayTag GetExperienceAttributeTag();
	static FGameplayTag GetHeroRoleTag();
	
	static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	static float GetStaticCostForAbility(const UGameplayAbility* Ability);

	static bool IsHero(const AActor* ActorToCheck);
	static bool IsAbilityAtMaxLevel(const FGameplayAbilitySpec& Spec);

	static bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec,const UAbilitySystemComponent& ASC);
	static float GetManaCostFor(const UGameplayAbility* AbilityCDO,const UAbilitySystemComponent&ASC,int AbilityLevel);
	static float GetCoolDownDurationFor(const UGameplayAbility* AbilityCDO,const UAbilitySystemComponent&ASC,int AbilityLevel);
	static float GetCoolDownRemainingFor(const UGameplayAbility* AbilityCDO,const UAbilitySystemComponent&ASC);
};
