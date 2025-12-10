// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "GAS/CAttributeSet.h"
#include "ValueGauge.generated.h"

class UAbilitySystemComponent;
/**
 * ValueGauge  属性条UI
 */
UCLASS()
class UValueGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	//相当于Widget的构造函数，初始化Widget
	virtual void NativePreConstruct() override;
	
	//在指定ASC中找到两个Value，绑定属性值变化委托回调函数，根据值变化更新Percent
	void SetAndBoundToGameplayAttribute(UAbilitySystemComponent* AbilitySystemComponent,const FGameplayAttribute& Attribute,const FGameplayAttribute& MaxAttribute);

	//属性值变化后回调调用，计算并更新Percent，更新Text
	void SetValue(float NewValue,float NewMaxValue);
private:
	//SetAndBoundToGameplayAttribute中两个值绑定的回调
	void ValueChanged(const FOnAttributeChangeData& ChangeData);
	void MaxValueChanged(const FOnAttributeChangeData& ChangeData);

	float CacheValue;
	float CacheMaxValue;
	
	UPROPERTY(EditAnywhere,Category="Visual")
	FLinearColor BarColor;

	UPROPERTY(EditAnywhere,Category="Visual")
	FSlateFontInfo ValueTextFont;

	UPROPERTY(EditAnywhere,Category="Visual")
	bool bValueTextVisible=true;

	UPROPERTY(EditAnywhere,Category="Visual")
	bool bProgressBarVisible=true;
	
	UPROPERTY(VisibleAnywhere,meta=(BindWidget))
	class UProgressBar* ProgressBar;
	
	UPROPERTY(VisibleAnywhere,meta=(BindWidget))
	class UTextBlock* ValueText;
	
	
};
