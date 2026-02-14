// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/AbilityGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "LevelGauge.h"
#include "Abilities/GameplayAbility.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "GAS/CHeroAttributeSet.h"
#include "GAS/UCAbilitySystemStatics.h"

void UAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct();

	//只有开始冷却才显示倒计时数值
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);

	UAbilitySystemComponent* OwnerASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	if (OwnerASC)
	{
		//当Cost/Cooldown调用时触发的委托
		OwnerASC->AbilityCommittedCallbacks.AddUObject(this,&ThisClass::AbilityCommitted);
		
		//在UpGradeGA中Spec被修改并且进行Dirt，对DirtSpec进行广播，这里进行相应，修改GALevel的UI
		OwnerASC->AbilitySpecDirtiedCallbacks.AddUObject(this,&ThisClass::AbilitySpecUpdated);

		//当UpgradePoint和Mana值变化时对应的回调
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetUpgradePointAttribute()).AddUObject(this,&ThisClass::UpgradePointUpdated);
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this,&ThisClass::ManaUpdated);

		//初始化UpgradePoint
		bool bFound=false;
		const float UpgradePoint=OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetUpgradePointAttribute(),bFound);
		if (bFound)
		{
			FOnAttributeChangeData ChangeData;
			ChangeData.NewValue=UpgradePoint;
			UpgradePointUpdated(ChangeData);
		}
	}
	//缓存，其他函数也会调用ASC
	OwnerASCComp=OwnerASC;

	//整数与带两个小数点的文本Op设置
	WholeNumberFormattingOptions.MaximumFractionalDigits=0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits=2;
}

void UAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	//从TSubClass到具体类，获得GA类模板实例CDO
	AbilityCDO=Cast<UGameplayAbility>(ListItemObject);

	//获取这个GA配置的GE对应的数值
	const float CooldownDuration=UCAbilitySystemStatics::GetStaticCooldownDurationForAbility(AbilityCDO);
	const float Cost=UCAbilitySystemStatics::GetStaticCostForAbility(AbilityCDO);

	//设置Text，初始化LevelGauge，最开始是0级
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	CostText->SetText(FText::AsNumber(Cost));
	LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(AbilityLevelParaName,0);
}

void UAbilityGauge::ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData)
{
	//将WidgetData中的SoftObjectIcon加载，随后作为Para的值
	if (Icon && WidgetData)
	{
		Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParaName,WidgetData->Icon.LoadSynchronous());
	}
}

void UAbilityGauge::AbilityCommitted(UGameplayAbility* Ability)
{
	//找到当前激活GA匹配的Gauge
	if (Ability->GetClass()->GetDefaultObject() ==AbilityCDO)
	{
		//这两个值用局部变量定义，因为需要多次调用此函数，每次都是初始化为0开始
		float CooldownTimeRemaining=0.0f;
		float CooldownDuration=0.0f;

		//获得具体值
		Ability->GetCooldownTimeRemainingAndDuration(Ability->GetCurrentAbilitySpecHandle(),Ability->GetCurrentActorInfo(),CooldownTimeRemaining,CooldownDuration);

		//开始冷却
		StartCooldown(CooldownTimeRemaining,CooldownDuration);
	}
}

void UAbilityGauge::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	//为冷却持续时间文本确定值
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));

	//缓存
	CacheCooldownDuration=CooldownDuration;
	CacheCooldownTimeRemaining=CooldownTimeRemaining;

	//倒计时显示值
	CooldownCounterText->SetVisibility(ESlateVisibility::Visible);

	//设置计时器
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle,this,&ThisClass::CooldownFinished,CacheCooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerUpdateHandle,this,&ThisClass::UpdateCooldown,CooldownUpdateInterval,true,0.f);
	
}

void UAbilityGauge::CooldownFinished()
{
	//冷却时间完成后将缓存值重置为0留给下次用
	CacheCooldownDuration=CacheCooldownTimeRemaining=0.f;

	//隐藏倒计时
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);

	//手动移除，因为设置了定时器循环
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
}

void UAbilityGauge::UpdateCooldown()
{
	//利用缓存好的值进行倒计时
	CacheCooldownTimeRemaining-=CooldownUpdateInterval;

	//设置值格式
	FNumberFormattingOptions* FormattingOptions=CacheCooldownTimeRemaining>1 ? &WholeNumberFormattingOptions : &TwoDigitNumberFormattingOptions;
	CooldownCounterText->SetText(FText::AsNumber(CacheCooldownTimeRemaining,FormattingOptions));

	//加上一个根据倒计时变化的Dynamic材质参数
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParaName,1.0f-CacheCooldownTimeRemaining/CacheCooldownDuration);
}

const FGameplayAbilitySpec* UAbilityGauge::GetAbilitySpec()
{
	if (!OwnerASCComp) return nullptr;
	if (!AbilityCDO) return nullptr;
	if (!CacheAbilitySpecHandle.IsValid())
	{
		//由于Spec指针被存储在动态数组特性的数据结构中，增减Spec都会影响容器的地址，如果缓存Spec指针，其指向可能由于容器的移动而为空
		//因此存储SpecHandle，即一个GA对应的唯一的Id
		FGameplayAbilitySpec* FoundAbilitySpec=OwnerASCComp->FindAbilitySpecFromClass(AbilityCDO->GetClass());
		CacheAbilitySpecHandle = FoundAbilitySpec->Handle;
		return FoundAbilitySpec;
	}

	return OwnerASCComp->FindAbilitySpecFromHandle(CacheAbilitySpecHandle);
}

void UAbilityGauge::AbilitySpecUpdated(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability != AbilityCDO) return ;

	bIsAbilityLearned=AbilitySpec.Level>0;

	LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(AbilityLevelParaName,AbilitySpec.Level);

	UpdateCanCast();

	const float NewCooldownDuration=UCAbilitySystemStatics::GetCoolDownDurationFor(AbilitySpec.Ability,*OwnerASCComp,AbilitySpec.Level);
	const float NewCost=UCAbilitySystemStatics::GetManaCostFor(AbilitySpec.Ability,*OwnerASCComp,AbilitySpec.Level);

	CooldownDurationText->SetText(FText::AsNumber(NewCooldownDuration));
	CostText->SetText(FText::AsNumber(NewCost));
}

void UAbilityGauge::UpdateCanCast()
{
	const FGameplayAbilitySpec* AbilitySpec=GetAbilitySpec();
	bool bCanCast=bIsAbilityLearned;
	
	if (AbilitySpec)
	{
		if (OwnerASCComp && !UCAbilitySystemStatics::CheckAbilityCost(*AbilitySpec,*OwnerASCComp))
		{
			bCanCast=false;
		}
	}
	
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CanCastAbilityParaName,bCanCast ? 1 :0);
}

void UAbilityGauge::UpgradePointUpdated(const FOnAttributeChangeData& Data)
{
	const bool HasUpgradePoint=Data.NewValue>0;

	const FGameplayAbilitySpec* AbilitySpec=GetAbilitySpec();

	if (AbilitySpec)
	{
		if (UCAbilitySystemStatics::IsAbilityAtMaxLevel(*AbilitySpec))
		{
			Icon->GetDynamicMaterial()->SetScalarParameterValue(UpgradePointAvailableParaName,0);
			return;
		}
	}
	Icon->GetDynamicMaterial()->SetScalarParameterValue(UpgradePointAvailableParaName,HasUpgradePoint ? 1:0);
}

void UAbilityGauge::ManaUpdated(const FOnAttributeChangeData& Data)
{
	UpdateCanCast();
}