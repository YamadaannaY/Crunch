// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ItemWidget.h"

#include "Components/Image.h"
#include "Crunch/DebugHelper.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//处理鼠标焦点
	SetIsFocusable(true);
	
}

void UItemWidget::SetIcon(UTexture2D* IconTexture)
{
	ItemIcon->SetBrushFromTexture(IconTexture);
}

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply SuperReply=Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	//将当前Widget设置为焦点（鼠标可操作区域）
	
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
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
	Debug::Print("RightMouse Button Clicked");
}

void UItemWidget::LeftButtonClicked()
{
	Debug::Print("LeftMouse Button Clicked");
	
}
