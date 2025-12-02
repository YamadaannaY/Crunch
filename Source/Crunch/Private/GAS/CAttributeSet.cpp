// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/CAttributeSet.h"
#include "Net/UnrealNetwork.h"

void UCAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	//用于在类中注册所有需要网络同步的变量：定义引擎对变量复制的规则：哪些变量需要被复制，如何复制，什么时候复制

	//如下规则表明：无条件且一直来自这个类的这些属性
	
	DOREPLIFETIME_CONDITION_NOTIFY(UCAttributeSet,Health,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCAttributeSet,MaxHealth,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCAttributeSet,Mana,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCAttributeSet,MaxMana,COND_None,REPNOTIFY_Always);
}

/**	GAMEPLAYATTRIBUTE_REPNOTIFY:主要触发OnAttributeChanged，广播当前属性和这个属性被修改前的值	 **/

void UCAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCAttributeSet,Health,OldValue);
}

void UCAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCAttributeSet,MaxHealth,OldValue);
}

void UCAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCAttributeSet,Mana,OldValue);
}

void UCAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCAttributeSet,MaxMana,OldValue);
}
