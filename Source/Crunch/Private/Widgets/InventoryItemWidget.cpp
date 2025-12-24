// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemWidget.h"

#include "ItemToolTip.h"
#include "Inventory/InventoryItem.h"
#include "Components/TextBlock.h"
#include "Inventory/PA_ShopItem.h"

void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EmptySlot();
}

void UInventoryItemWidget::EmptySlot()
{
	InventoryItem=nullptr;
	SetIcon(EmptyTexture);

	SetToolTip(nullptr);

	StackCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	ManaCostText->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryItemWidget::UpdateInventoryItem(const UInventoryItem* Item)
{
	InventoryItem=Item;
	
	if (!InventoryItem || !InventoryItem->IsValid() || InventoryItem->GetStackCount()<=0)
	{
		EmptySlot();
		return ;
	}

	SetIcon(Item->GetShopItem()->GetIcon());

	UItemToolTip* ToolTip=SetToolTipWidget(InventoryItem->GetShopItem());
	if (ToolTip)
	{
		ToolTip->SetPrice(InventoryItem->GetShopItem()->GetSellPrice());
	}

	if (InventoryItem->GetShopItem()->GetIsStackable())
	{
		StackCountText->SetVisibility(ESlateVisibility::Visible);

		//更新StackText
		UpdateStackCount();
	}
	else
	{
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}
}

bool UInventoryItemWidget::IsEmpty() const
{
	return !InventoryItem || !InventoryItem->IsValid();
}

void UInventoryItemWidget::UpdateStackCount()
{
	if (InventoryItem)
	{
		StackCountText->SetText(FText::AsNumber(InventoryItem->GetStackCount()));
	}
}
