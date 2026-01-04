// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/UCAbilitySystemStatics.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"

FGameplayTag UCAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack");


}

FGameplayTag UCAbilitySystemStatics::GetBasicAttackInputPressedTag()
{
	//当InputID时basicattack时会用这个Tag作为EventTag来触发WaitEvent（UpperCut中）
	return FGameplayTag::RequestGameplayTag("ability.basicattack.pressed");
}

FGameplayTag UCAbilitySystemStatics::GetDeadStatTag()
{
	return FGameplayTag::RequestGameplayTag("stats.dead");
}

FGameplayTag UCAbilitySystemStatics::GetStunStatTag()
{
	return FGameplayTag::RequestGameplayTag("stats.stun");
}

FGameplayTag UCAbilitySystemStatics::GetAimStatTag()
{
	return FGameplayTag::RequestGameplayTag("stats.aim");
}

FGameplayTag UCAbilitySystemStatics::GetHealthFullStatTag()
{
	return FGameplayTag::RequestGameplayTag("stats.health.full");
}

FGameplayTag UCAbilitySystemStatics::GetHealthEmptyStatTag()
{
	return FGameplayTag::RequestGameplayTag("stats.health.empty");
}

FGameplayTag UCAbilitySystemStatics::GetManaFullStatTag()
{
	return FGameplayTag::RequestGameplayTag("stats.mana.full");
}

FGameplayTag UCAbilitySystemStatics::GetManaEmptyStatTag()
{
	return FGameplayTag::RequestGameplayTag("stats.mana.empty");
}

FGameplayTag UCAbilitySystemStatics::GetCameraShakeCueTag()
{
	return FGameplayTag::RequestGameplayTag("GameplayCue.cameraShake");
}

FGameplayTag UCAbilitySystemStatics::GetGoldAttributeTag()
{
	return FGameplayTag::RequestGameplayTag("attr.gold");
}

FGameplayTag UCAbilitySystemStatics::GetExperienceAttributeTag()
{
	return FGameplayTag::RequestGameplayTag("attr.experience");
}

FGameplayTag UCAbilitySystemStatics::GetHeroRoleTag()
{
	return FGameplayTag::RequestGameplayTag("role.hero");
}

float UCAbilitySystemStatics::GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability)
{
	if (!Ability) return 0.f;

	const UGameplayEffect* CooldownGE=Ability->GetCooldownGameplayEffect();
	if (!CooldownGE) return 0.f ;

	float CooldownDuration=0.f;

	CooldownGE->DurationMagnitude.GetStaticMagnitudeIfPossible(1,CooldownDuration);

	return CooldownDuration;
}

float UCAbilitySystemStatics::GetStaticCostForAbility(const UGameplayAbility* Ability)
{
	if (!Ability) return 0;

	const UGameplayEffect* CostGE=Ability->GetCostGameplayEffect();
	if (!CostGE || CostGE->Modifiers.Num()==0) return 0.f;

	float Cost=0.f;
	
	//Modifiers[0]==Mana
	CostGE->Modifiers[0].ModifierMagnitude.GetStaticMagnitudeIfPossible(1,Cost);

	return FMath::Abs(Cost);
}

bool UCAbilitySystemStatics::IsHero(const AActor* ActorToCheck)
{
	//不直接获取的原因是这里的Actor是常量，但是蓝图库中那个函数返回的不是常量
	const IAbilitySystemInterface* ActorISA=Cast<IAbilitySystemInterface>(ActorToCheck);

	if (ActorISA)
	{
		UAbilitySystemComponent* ActorASC=ActorISA->GetAbilitySystemComponent();

		if (ActorASC)
		{
			return ActorASC->HasMatchingGameplayTag(GetHeroRoleTag());
		}
	}
	return false;
}

bool UCAbilitySystemStatics::IsAbilityAtMaxLevel(const FGameplayAbilitySpec& Spec)
{
	return Spec.Level>=4;
}

bool UCAbilitySystemStatics::CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec,
	const UAbilitySystemComponent& ASC)
{
	const UGameplayAbility* AbilityCDO=AbilitySpec.Ability;
	if (AbilityCDO)
	{
		return AbilityCDO->CheckCost(AbilitySpec.Handle,ASC.AbilityActorInfo.Get());
	}
	
	return false;
}

bool UCAbilitySystemStatics::CheckAbilityCost(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC)
{
	if (AbilityCDO)
	{
		return AbilityCDO->CheckCost(FGameplayAbilitySpecHandle(),ASC.AbilityActorInfo.Get());
	}
	
	return false;
}

float UCAbilitySystemStatics::GetManaCostFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent&ASC,
                                             int AbilityLevel)
{
	float ManaCost=0.f;
	if (AbilityCDO)
	{
		UGameplayEffect* CostEffect=AbilityCDO->GetCostGameplayEffect();
		if (CostEffect)
		{
			FGameplayEffectSpecHandle EffectSpec=ASC.MakeOutgoingSpec(CostEffect->GetClass(),AbilityLevel,ASC.MakeEffectContext());
			CostEffect->Modifiers[0].ModifierMagnitude.AttemptCalculateMagnitude(*EffectSpec.Data.Get(),ManaCost);
		}
	}

	return FMath::Abs(ManaCost);
}

float UCAbilitySystemStatics::GetCoolDownDurationFor(const UGameplayAbility* AbilityCDO,
	const UAbilitySystemComponent&ASC, int AbilityLevel)
{
	float CoolDownDuration=0.f;
	if (AbilityCDO)
	{
		UGameplayEffect* CoolDownEffect=AbilityCDO->GetCooldownGameplayEffect();
		if (CoolDownEffect)
		{
			FGameplayEffectSpecHandle EffectSpec=ASC.MakeOutgoingSpec(CoolDownEffect->GetClass(),AbilityLevel,ASC.MakeEffectContext());
			CoolDownEffect->DurationMagnitude.AttemptCalculateMagnitude(*EffectSpec.Data.Get(),CoolDownDuration);
		}
	}
	return FMath::Abs(CoolDownDuration);
}

float UCAbilitySystemStatics::GetCoolDownRemainingFor(const UGameplayAbility* AbilityCDO,
	const UAbilitySystemComponent&ASC)
{
	if (!AbilityCDO)
	{
		return 0.f;
	}

	UGameplayEffect* CooldownEffect=AbilityCDO->GetCooldownGameplayEffect();
	if (!CooldownEffect) return 0.f;

	FGameplayEffectQuery CooldownEffectQuery;
	CooldownEffectQuery.EffectDefinition=CooldownEffect->GetClass();

	float CooldownRemaining=0.f;
	FJsonSerializableArrayFloat CooldownTimeRemainings=ASC.GetActiveEffectsTimeRemaining(CooldownEffectQuery);

	for (float Remaining : CooldownTimeRemainings)
	{
		if (Remaining>CooldownRemaining)
		{
			CooldownRemaining=Remaining;
		}
	}
	return CooldownRemaining;
}
