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
	static FGameplayTag GetBasicAttackInputReleasedTag();
	static FGameplayTag GetDeadStatTag();
	static FGameplayTag GetStunStatTag();
	static FGameplayTag GetAimStatTag();
	static FGameplayTag GetFocusStatTag();
	static FGameplayTag GetHealthFullStatTag();
	static FGameplayTag GetHealthEmptyStatTag();
	static FGameplayTag GetManaFullStatTag();
	static FGameplayTag GetManaEmptyStatTag();
	static FGameplayTag GetCameraShakeCueTag();
	static FGameplayTag GetGoldAttributeTag();
	static FGameplayTag GetExperienceAttributeTag();
	static FGameplayTag GetTargetUpdatedTag();
	static FGameplayTag GetHeroRoleTag();
	static FGameplayTag GetCrosshairTag();
	static FGameplayTag GetGenericDamagePointTag();
	static FGameplayTag GetGenericTargetPointTag();

	static bool IsActorDead(const AActor* ActorToCheck);
	static bool IsHero(const AActor* ActorToCheck);
	static bool ActorHasTag(const AActor* ActorToCheck,const FGameplayTag& Tag);
	static bool IsAbilityAtMaxLevel(const FGameplayAbilitySpec& Spec);
	
	//获取配置的初始Cooldown值
	static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	
	//获取配置的初始CostGE值
	static float GetStaticCostForAbility(const UGameplayAbility* Ability);
	
	//通过Spec判断是否可以释放GA
	static bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec,const UAbilitySystemComponent& ASC);
	
	//通过GA实例判断是否可以释放GA
	static bool CheckAbilityCost(const UGameplayAbility* AbilityCDO,const UAbilitySystemComponent& ASC);
	
	//获取当前GA的Mana消耗值
	static float GetManaCostFor(const UGameplayAbility* AbilityCDO,const UAbilitySystemComponent&ASC,int AbilityLevel);
	
	//获取当前GA的冷却时间
	static float GetCoolDownDurationFor(const UGameplayAbility* AbilityCDO,const UAbilitySystemComponent&ASC,int AbilityLevel);
	
	//获取当前GA的剩余时间
	static float GetCoolDownRemainingFor(const UGameplayAbility* AbilityCDO,const UAbilitySystemComponent&ASC);
	
	//在客户端本地执行GameplayCue
	static void SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult, const FGameplayTag& GameplayCueTag);
};
