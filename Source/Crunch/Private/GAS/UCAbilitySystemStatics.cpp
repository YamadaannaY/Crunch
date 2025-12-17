// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/UCAbilitySystemStatics.h"
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
