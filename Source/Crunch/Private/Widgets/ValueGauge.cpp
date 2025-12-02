// Fill out your copyright notice in the Description page of Project Settings.


#include "ValueGauge.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UValueGauge::NativePreConstruct()
{
	Super::NativePreConstruct();

	//为Bar提供颜色
	ProgressBar->SetFillColorAndOpacity(BarColor);
}

void UValueGauge::SetAndBoundToGameplayAttribute(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute)
{
	if (AbilitySystemComponent)
	{
		bool bFound;
		
		//在指定ASC中寻找两个Value，如果找到进行一次初始化，随后正式开始追踪UI变化
		float Value=AbilitySystemComponent->GetGameplayAttributeValue(Attribute,bFound);
		float MaxValue=AbilitySystemComponent->GetGameplayAttributeValue(MaxAttribute,bFound);
		if(bFound)
		{
			
			SetValue(Value,MaxValue);
		}
		
		//两个属性的值改变时回调的函数
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this,&ThisClass::ValueChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this,&ThisClass::MaxValueChanged);
	}
}

void UValueGauge::SetValue(float NewValue, float NewMaxValue)
{
	//时刻更新改变后的值，实际上是这个函数被调用时的参数
	CacheValue=NewValue;
	CacheMaxValue=NewMaxValue;

	//NewMaxValue作为分母不能为0
	if (NewMaxValue==0)
	{
		UE_LOG(LogTemp,Warning,TEXT("ValueGauge:%s,NewMaxValue can`t be 0"),*GetName());
		return;
	}

	//改变Bar的Percent
	float NewPercent=NewValue/NewMaxValue;
	ProgressBar->SetPercent(NewPercent);

	//ValueText的设置，设置没有小数点，并且规定了Text的格式，传入值
	FNumberFormattingOptions FormatOps=FNumberFormattingOptions().SetMaximumFractionalDigits(0);
	FText NewText = FText::Format(
	FTextFormat::FromString("{0}/{1}"),
	FText::AsNumber(NewValue, &FormatOps),
	FText::AsNumber(NewMaxValue, &FormatOps)
);
	ValueText->SetText(NewText);
}

void UValueGauge::ValueChanged(const FOnAttributeChangeData& ChangeData)
{
	//当Value改变时
	SetValue(ChangeData.NewValue,CacheMaxValue);
}

void UValueGauge::MaxValueChanged(const FOnAttributeChangeData& ChangeData)
{
	//当MaxValue改变时
	SetValue(CacheValue,ChangeData.NewValue);
}
