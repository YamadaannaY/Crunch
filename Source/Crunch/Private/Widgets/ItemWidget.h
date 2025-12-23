// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemWidget.generated.h"

class UPA_ShopItem;
class UItemToolTip;
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
protected:
	UItemToolTip* SetToolTipWidget(const UPA_ShopItem* Item);
private:	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void RightButtonClicked();
	virtual void LeftButtonClicked();
	
	UPROPERTY(meta=(BindWidget))
	UImage* ItemIcon;
	
	UPROPERTY(EditDefaultsOnly,Category="Tool Tip")
	TSubclassOf<UItemToolTip> ToolTipWidgetClass;
};
