// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UCAbilitySystemStatics.h"

FGameplayTag UCAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("ability.basicattack");


}
