// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemComponent.h"
#include "Crunch/DebugHelper.h"
#include "GAS/CAttributeSet.h"

UCAbilitySystemComponent::UCAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(this,&ThisClass::HealthUpdated);
}

void UCAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return ;
	
	for (const TSubclassOf<UGameplayEffect>& Effect : InitialEffects)
	{
		//应用自身直接用默认的EffectContext
		FGameplayEffectSpecHandle EffectSpecHandle=MakeOutgoingSpec(Effect,1,MakeEffectContext());
		
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return ;

	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>> AbilityPair: Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,0,(int32)AbilityPair.Key,nullptr));
	}
	
	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>> AbilityPair: BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,0,(int32)AbilityPair.Key,nullptr));
	}
}

void UCAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner()) return ;

	if (ChangeData.NewValue<=0 && GetOwner()->HasAuthority() && DeathEffect)
	{
		FGameplayEffectSpecHandle EffectSpecHandle=MakeOutgoingSpec(DeathEffect,1,MakeEffectContext());
		
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}
