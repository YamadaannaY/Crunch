// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/CAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CHeroAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "UCAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"
#include "PA_AbilitySystemGeneric.h"
#include "Crunch/DebugHelper.h"

UCAbilitySystemComponent::UCAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetHealthAttribute()).AddUObject(this,&ThisClass::HealthUpdated);
	GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this,&ThisClass::ManaUpdated);
	GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this,&ThisClass::ExperienceUpdated);

	//为Confirm/Cancel 添加InputID
	GenericConfirmInputID=(int32) ECAbilityInputID::Confirm;
	GenericCancelInputID=(int32) ECAbilityInputID::Cancel;
}

void UCAbilitySystemComponent::InitializeBaseAttribute()
{
	if (!AbilitySystemGeneric || !AbilitySystemGeneric->GetBaseStatsDataTable() || !GetOwner()) return ;

	//存储了所有Hero各项属性的DT
	const UDataTable* BaseStatsDataTable=AbilitySystemGeneric->GetBaseStatsDataTable();
	
	const FHeroBaseStats* BaseStats=nullptr;

	//遍历DT，找到当前ASC拥有者的属性行，将其赋予BaseStats
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
		SetNumericAttributeBase(UCHeroAttributeSet::GetGoldAttribute(),BaseStats->BaseGold);
	}

	//一个曲线表Table存储了所有等级的经验数值
	const FRealCurve* ExperienceCurve=AbilitySystemGeneric->GetExperienceCurve();
	if (ExperienceCurve)
	{
		//Key的数量即最大等级
		const int MaxLevel=ExperienceCurve->GetNumKeys();
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelAttribute(),MaxLevel);

		//最大经验值，即最后一个Key对应的Value
		const float MaxExp=ExperienceCurve->GetKeyValue(ExperienceCurve->GetLastKeyHandle());
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelExperienceAttribute(),MaxExp);
	}

	//调用一次Exp更新进行初始化，传参为0，即最开始Exp值为0
	ExperienceUpdated(FOnAttributeChangeData());
}

void UCAbilitySystemComponent::ServerSideInit()
{
	InitializeBaseAttribute();
	ApplyInitialEffects();
	GiveInitialAbilities();
}

void UCAbilitySystemComponent::Client_AbilitySpecLevelUpdated_Implementation(FGameplayAbilitySpecHandle Handle,
	int Level)
{
	FGameplayAbilitySpec* Spec=FindAbilitySpecFromHandle(Handle);
	if (Spec)
	{
		Spec->Level=Level;
		
		//广播SpecDirtied
		AbilitySpecDirtiedCallbacks.Broadcast(*Spec);
	}
}

void UCAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return ;

	if (!AbilitySystemGeneric) return;
	
	for (const TSubclassOf<UGameplayEffect>& Effect : AbilitySystemGeneric->GetInitialEffects())
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
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,0,(int32)AbilityPair.Key,nullptr));
	}
	
	for (const TPair<ECAbilityInputID, TSubclassOf<UGameplayAbility>> AbilityPair: BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value,1,(int32)AbilityPair.Key,nullptr));
	}

	if (!AbilitySystemGeneric) return;
	
	for (const TSubclassOf<UGameplayAbility>& PassiveAbility :AbilitySystemGeneric->GetPassiveAbilities())
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
	if (!AbilitySystemGeneric) return;
	
	AuthApplyGameplayEffect(AbilitySystemGeneric->GetFullStateEffect());
}

const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& UCAbilitySystemComponent::GetAbilities() const
{
	return Abilities;
}

bool UCAbilitySystemComponent::IsAtMaxLevel() const
{
	bool bFound;

	//判断当前Level值是否与MaxLevel值相等
	const float CurrentLevel=GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(),bFound);
	const float CurrentMaxLevel=GetGameplayAttributeValue(UCHeroAttributeSet::GetMaxLevelAttribute(),bFound);

	if (! bFound)
	{
		Debug::Print(TEXT("Don`t have Level or MaxLevel Value !"));
		return false;
	}

	return CurrentLevel>=CurrentMaxLevel;
}

void UCAbilitySystemComponent::Server_UpgradeAbilityWithID_Implementation(ECAbilityInputID InputID)
{
	bool bFound=false;

	const float UpgradePoint=GetGameplayAttributeValue(UCHeroAttributeSet::GetUpgradePointAttribute(),bFound);
	if (!bFound || UpgradePoint<=0) return ;
	
	FGameplayAbilitySpec* AbilitySpec=FindAbilitySpecFromInputID((int32)InputID);
	if (! AbilitySpec || UCAbilitySystemStatics::IsAbilityAtMaxLevel(*AbilitySpec)) return;

	//一个升级点使GA升一级
	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(),UpgradePoint-1);
	AbilitySpec->Level+=1;

	//Call to mark that an ability spec has been modified
	MarkAbilitySpecDirty(*AbilitySpec);

	Client_AbilitySpecLevelUpdated(AbilitySpec->Handle,AbilitySpec->Level);
}

bool UCAbilitySystemComponent::Server_UpgradeAbilityWithID_Validate(ECAbilityInputID InputID)
{
	return true;
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


			if (AbilitySystemGeneric && AbilitySystemGeneric->GetDeathEffect())
			{
				AuthApplyGameplayEffect(AbilitySystemGeneric->GetDeathEffect());

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

void UCAbilitySystemComponent::ExperienceUpdated(const FOnAttributeChangeData& ChangeData)
{
	//前置条件
	if (!GetOwner() || !GetOwner()->HasAuthority()) return ;
	if (IsAtMaxLevel()) return;
	if (!AbilitySystemGeneric) return ;

	//将参数值作为当前Exp值不断更新
	const float CurrentExp=ChangeData.NewValue;

	//经验值行
	const FRealCurve* ExperienceCurve=AbilitySystemGeneric->GetExperienceCurve();

	if (!ExperienceCurve)
	{
		Debug::Print("can`t find experience data !!");
		return;
	}
	
	float PrevLevelExp=0;
	float NextLevelExp=0;
	float NewLevel=1;

	//遍历Key的Iter迭代器
	for (auto Iter=ExperienceCurve->GetKeyHandleIterator();Iter;++Iter)
	{
		//当前等级对应的具体经验值
		const float ExperienceToReachLevel=ExperienceCurve->GetKeyValue(*Iter);

		//当前Exp没有达到升级经验值，将这个升级经验值存储为属性值NextLevelExperience
		if (CurrentExp<ExperienceToReachLevel)
		{
			NextLevelExp=ExperienceToReachLevel;
			break;
		}

		//当前经验值大于遍历到Level的Exp，将其记录为PreLevelExp属性值
		PrevLevelExp=ExperienceToReachLevel;
		//等级+1
		NewLevel=Iter.GetIndex()+1;
	}

	//获取原本属性集值
	const float CurrentLevel=GetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute());
	const float CurrentUpGradePoint=GetNumericAttribute(UCHeroAttributeSet::GetUpgradePointAttribute());

	//升级点，用于提升GA等级
	const float LevelUpgrade=NewLevel-CurrentLevel;
	const float NewUpgradePoint=CurrentUpGradePoint+LevelUpgrade;

	//为属性集赋值
	SetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute(),NewLevel);
	SetNumericAttributeBase(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(),PrevLevelExp);
	SetNumericAttributeBase(UCHeroAttributeSet::GetNextLevelExperienceAttribute(),NextLevelExp);
	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(),NewUpgradePoint);
}