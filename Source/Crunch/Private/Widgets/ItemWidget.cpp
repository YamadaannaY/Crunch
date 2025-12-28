// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/ItemWidget.h"
#include "Widgets/ItemToolTip.h"
#include "Components/Image.h"


void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//允许聚焦，注意不是设置为聚焦
	SetIsFocusable(true);
}

void UItemWidget::SetIcon(UTexture2D* IconTexture)
{
	ItemIcon->SetBrushFromTexture(IconTexture);
}

UItemToolTip* UItemWidget::SetToolTipWidget(const UPA_ShopItem* Item)
{
	if (!Item) return nullptr;

	if (GetOwningPlayer() && ToolTipWidgetClass)
	{
		UItemToolTip* ToolTip=CreateWidget<UItemToolTip>(GetOwningPlayer(),ToolTipWidgetClass);

		if (ToolTip)
		{
			//为Tip设置参数
			ToolTip->SetItem(Item);
			
			//引擎会自动检测鼠标悬停，并在鼠标进入区域时显示该Widget，移出时隐藏。
			SetToolTip(ToolTip);
		}
		return ToolTip;
	}
	return nullptr;
}

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply SuperReply=Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	//将当前Widget设置为焦点（鼠标可操作区域），Handled()表示点击事件处理完毕，不再处理这个输入，防止误触其他操作
	
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		//将当前焦点转移到这个Item上
		return FReply::Handled().SetUserFocus(TakeWidget());
	}

	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().SetUserFocus(TakeWidget()).DetectDrag(TakeWidget(),EKeys::LeftMouseButton);
	}

	return SuperReply;
}

FReply UItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply SuperReply=Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	//ButtonDown中设置SetFocus，在这里判断
	if (HasAnyUserFocus())
	{
		if (InMouseEvent.GetEffectingButton()==EKeys::RightMouseButton)
		{
			RightButtonClicked();
			return FReply::Handled();
		}
		if (InMouseEvent.GetEffectingButton()==EKeys::LeftMouseButton)
		{
			LeftButtonClicked();
			return FReply::Handled();
		}
	}
	return SuperReply;
}

void UItemWidget::RightButtonClicked()
{
	
}

void UItemWidget::LeftButtonClicked()
{
	
}
