// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/StatsGauge.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

void UStatsGauge::NativePreConstruct()
{
	Super::NativePreConstruct();

	//将Image用Texture填充
	Icon->SetBrushFromTexture(IconTexture);
}

void UStatsGauge::NativeConstruct()
{
	Super::NativeConstruct();

	//不需要小数点
	FormattingOptions.MaximumFractionalDigits=0;
	
	APawn* OwnerPlayerPawn=GetOwningPlayerPawn();
	if (!OwnerPlayerPawn) return ;
	UAbilitySystemComponent* OwnerASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPlayerPawn);

	if(OwnerASC)
	{
		bool bFound;
		
		//为GaugeText赋值
		float AttributeVal=OwnerASC->GetGameplayAttributeValue(Attribute,bFound);
		SetValue(AttributeVal);

		//监听对应属性的变化
		OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this,&ThisClass::AttributeChanged);
	}
}

void UStatsGauge::SetValue(float NewValue)
{
	//改变GaugeText
	AttributeText->SetText(FText::AsNumber(NewValue,&FormattingOptions));
}

void UStatsGauge::AttributeChanged(const FOnAttributeChangeData& Data)
{
	SetValue(Data.NewValue);
}
