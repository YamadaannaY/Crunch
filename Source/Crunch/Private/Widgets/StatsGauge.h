// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "StatsGauge.generated.h"

/**
 * 
 */
struct FOnAttributeChangeData;


UCLASS()
class UStatsGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	//这里调用的函数会在编辑器中可见，如Text、Image，蓝图进行预览
	virtual void NativePreConstruct() override;

	//主要构建逻辑
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* AttributeText;

	UPROPERTY(EditAnywhere,Category="Visual")
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere,Category="Visual")
	UTexture2D* IconTexture;

	//根据传入Value改变Text
	void SetValue(float NewValue);
	
	//属性值改变调用函数
	void AttributeChanged(const FOnAttributeChangeData& Data);

	FNumberFormattingOptions FormattingOptions;
};
