// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemComponent.h"

void UCAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return ;
	
	for (const TSubclassOf<UGameplayEffect>& Effect : InitialEffects)
	{
		//应用自身直接用默认的EffectContext
		FGameplayEffectSpecHandle EffectSpecHandle=MakeOutgoingSpec(Effect,1,MakeEffectContext());
		
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
	}
}
