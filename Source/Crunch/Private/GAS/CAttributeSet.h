// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Attribute Set
 */
UCLASS()
class UCAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	//目的是使用宏定义的函数
	ATTRIBUTE_ACCESSORS(UCAttributeSet,Health);
	ATTRIBUTE_ACCESSORS(UCAttributeSet,MaxHealth);
	ATTRIBUTE_ACCESSORS(UCAttributeSet,Mana);
	ATTRIBUTE_ACCESSORS(UCAttributeSet,MaxMana);
	ATTRIBUTE_ACCESSORS(UCAttributeSet,Armor);
	ATTRIBUTE_ACCESSORS(UCAttributeSet,AttackDamage);
	ATTRIBUTE_ACCESSORS(UCAttributeSet,MoveSpeed);

	//如果一个变量声明了UPROPERTY(Replicated)，但没有加入GetLifetimeReplicatedProps()，不会进行同步,因为缺少同步规则。
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	//处理由代码直接修改属性值的情况，会在属性值改变前执行
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	//处理所有由GE修改的属性值，会在属性值改变后执行
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
private:
	/**ReplicatedUsing表示服务端复制这个宏标记的属性到客户端，同时调用OnRep函数 **/
	
	UPROPERTY(ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	UPROPERTY(ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	
	UPROPERTY(ReplicatedUsing=OnRep_Mana)
	FGameplayAttributeData Mana;
	UPROPERTY(ReplicatedUsing=OnRep_MaxMana)
	FGameplayAttributeData MaxMana;

	UPROPERTY(ReplicatedUsing=OnRep_AttackDamage)
	FGameplayAttributeData AttackDamage;

	UPROPERTY(ReplicatedUsing=OnRep_Armor)
	FGameplayAttributeData Armor;

	UPROPERTY(ReplicatedUsing=OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
};
