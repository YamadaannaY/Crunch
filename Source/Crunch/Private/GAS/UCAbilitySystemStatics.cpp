// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UCAbilitySystemStatics.h"

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
