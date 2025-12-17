// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/AbilityGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/CAbilitySystemComponent.h"
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
	}

	WholeNumberFormattingOptions.MaximumFractionalDigits=0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits=2;
}

void UAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	//在ListItem中AddItem的对象就是SubClassOfGA，获取其CDO
	AbilityCDO=Cast<UGameplayAbility>(ListItemObject);

	//获取这个CDO配置的GE对应的数值
	const float CooldownDuration=UCAbilitySystemStatics::GetStaticCooldownDurationForAbility(AbilityCDO);
	const float Cost=UCAbilitySystemStatics::GetStaticCostForAbility(AbilityCDO);

	//设置Text
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	CostText->SetText(FText::AsNumber(Cost));
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
	//确认调用GA类默认对象就是这个Gauge存储的GA CDO 用CDO的原因：在ListItem中传入的是GASubClassof，即一个默认类对象而非实例
	if (Ability->GetClass()->GetDefaultObject() ==AbilityCDO)
	{
		//这两个值用局部变量定义，因为需要多次调用此函数，每次都是初始化为0
		float CooldownTimeRemaining=0.0f;
		float CooldownDuration=0.0f;

		//获得具体值
		Ability->GetCooldownTimeRemainingAndDuration(Ability->GetCurrentAbilitySpecHandle(),Ability->GetCurrentActorInfo(),CooldownTimeRemaining,CooldownDuration);

		//开始冷却逻辑
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

	//加上一个倒计时渲染
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParaName,1.0f-CacheCooldownTimeRemaining/CacheCooldownDuration);
}
