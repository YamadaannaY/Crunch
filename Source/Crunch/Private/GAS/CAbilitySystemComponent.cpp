// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/CAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CHeroAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "UCAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"

UCAbilitySystemComponent::UCAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(this,&ThisClass::HealthUpdated);
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this,&ThisClass::ManaUpdated);
	//为Confirm/Cancel 添加InputID
	GenericConfirmInputID=(int32) ECAbilityInputID::Confirm;
	GenericCancelInputID=(int32) ECAbilityInputID::Cancel;
}

void UCAbilitySystemComponent::InitializeBaseAttribute()
{
	//找到DT中对应于ASC组件拥有者的Class
	if (!BaseStatsDataTable || !GetOwner()) return ;

	const FHeroBaseStats* BaseStats=nullptr;

	for (const TPair<FName,uint8*>& DataPair : BaseStatsDataTable->GetRowMap())
	{
		BaseStats=BaseStatsDataTable->FindRow<FHeroBaseStats>(DataPair.Key,"");

		if (BaseStats && BaseStats->Class==GetOwner()->GetClass())
		{
			break;
		}
	}

	//将BT各项数值应用到属性集对应的属性中
	if (BaseStats)
	{
		SetNumericAttributeBase(UCAttributeSet::GetMaxHealthAttribute(),BaseStats->BaseMaxHealth);
		SetNumericAttributeBase(UCAttributeSet::GetMaxManaAttribute(),BaseStats->BaseMaxMana);
		SetNumericAttributeBase(UCAttributeSet::GetAttackDamageAttribute(),BaseStats->BaseAttackDamage);
		SetNumericAttributeBase(UCAttributeSet::GetArmorAttribute(),BaseStats->BaseArmor);
		SetNumericAttributeBase(UCAttributeSet::GetMoveSpeedAttribute(),BaseStats->BaseMoveSpeed);

		//Hero专属
		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthAttribute(),BaseStats->Strength);
		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceAttribute(),BaseStats->Intelligence);
		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthGrowthRateAttribute(),BaseStats->StrengthGrowthRate);
		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceGrowthRateAttribute(),BaseStats->IntelligenceGrowthRate);
	}
}

void UCAbilitySystemComponent::ServerSideInit()
{
	InitializeBaseAttribute();
	ApplyInitialEffects();
	GiveInitialAbilities();
}

void UCAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return ;
	
	for (const TSubclassOf<UGameplayEffect>& Effect : InitialEffects)
	{
		//应用自身直接用默认的EffectContext
		AuthApplyGameplayEffect(Effect);
	}
}

void UCAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return ;

	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>> AbilityPair: Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,1,(int32)AbilityPair.Key,nullptr));
	}
	
	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>> AbilityPair: BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,1,(int32)AbilityPair.Key,nullptr));
	}

	for (const TSubclassOf<UGameplayAbility>& PassiveAbility : PassiveAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(PassiveAbility,1,-1,nullptr));
	}
}

void UCAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle=MakeOutgoingSpec(GameplayEffect,Level,MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAbilitySystemComponent::ApplyFullStatsEffect()
{
	AuthApplyGameplayEffect(FullStatEffect);
}

const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& UCAbilitySystemComponent::GetAbilities() const
{
	return Abilities;
}

void UCAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() ) return ;

	bool bFound=false;
	float MaxHealth=GetGameplayAttributeValue(UCAttributeSet::GetMaxHealthAttribute(),bFound);
	if (bFound && ChangeData.NewValue>=MaxHealth)
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetHealthFullStatTag()))
		{
			//do local only
			AddLooseGameplayTag(UCAbilitySystemStatics::GetHealthFullStatTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetHealthFullStatTag());
	}
	
	if (ChangeData.NewValue<=0)
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetHealthEmptyStatTag()))
		{
			AddLooseGameplayTag(UCAbilitySystemStatics::GetHealthEmptyStatTag());
			if (DeathEffect)
			{
				AuthApplyGameplayEffect(DeathEffect);

				FGameplayEventData DeadAbilityEventData;
				if (ChangeData.GEModData)
				{
					DeadAbilityEventData.ContextHandle=ChangeData.GEModData->EffectSpec.GetContext();
				}

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(),UCAbilitySystemStatics::GetDeadStatTag(),DeadAbilityEventData);
			}
		}
	}
	else RemoveLooseGameplayTag(UCAbilitySystemStatics::GetHealthEmptyStatTag());
}

void UCAbilitySystemComponent::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() ) return ;

	bool bFound=false;
	float MaxMana=GetGameplayAttributeValue(UCAttributeSet::GetMaxManaAttribute(),bFound);
	if (bFound && ChangeData.NewValue>=MaxMana)
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetManaFullStatTag()))
		{
			//do local only
			AddLooseGameplayTag(UCAbilitySystemStatics::GetManaFullStatTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UCAbilitySystemStatics::GetManaFullStatTag());
	}
	
	if (ChangeData.NewValue<=0)
	{
		if (!HasMatchingGameplayTag(UCAbilitySystemStatics::GetManaEmptyStatTag()))
		{
			AddLooseGameplayTag(UCAbilitySystemStatics::GetManaEmptyStatTag());
		}
	}
	else RemoveLooseGameplayTag(UCAbilitySystemStatics::GetHealthEmptyStatTag());
}
