// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemWidget.generated.h"

class UImage;

/**
 * Item的Widget基类
 */
UCLASS()
class UItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	//SetBrush
	virtual void SetIcon(UTexture2D* IconTexture);

	//对鼠标输入进行判定，管理焦点变化

	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void RightButtonClicked();
	virtual void LeftButtonClicked();

private:
	UPROPERTY(meta=(BindWidget))
	UImage* ItemIcon;
};
