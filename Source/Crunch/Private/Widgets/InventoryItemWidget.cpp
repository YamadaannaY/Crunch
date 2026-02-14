// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemWidget.h"
#include  "Widgets/InventoryItemDragDropOp.h"
#include "ItemToolTip.h"
#include "Components/Image.h"
#include "Inventory/InventoryItem.h"
#include "Components/TextBlock.h"
#include "Inventory/PA_ShopItem.h"

void UInventoryItemWidget::RightButtonClicked()
{
	if (!IsEmpty())
	{
		OnRightButtonClick.Broadcast(GetItemHandle());
	}
}

void UInventoryItemWidget::LeftButtonClicked()
{
	if (!IsEmpty())
	{
		OnLeftButtonClick.Broadcast(GetItemHandle());
	}
}

void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	EmptySlot();
}

void UInventoryItemWidget::EmptySlot()
{
	ClearCooldown();
	UnBindCanCastAbilityDelegate();
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
	UnBindCanCastAbilityDelegate();
	
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

	ClearCooldown();

	if (InventoryItem->IsGrantingAnyAbility())
	{
		UpdateCanCast(InventoryItem->CanCastAbility());
		float AbilityCooldownRemaining=InventoryItem->GetAbilityCooldownTimeRemaining();
		float AbilityCooldownDuration=InventoryItem->GetAbilityCooldownDuration();

		UE_LOG(LogTemp,Warning,TEXT("remaining:%f,duration:%f"),AbilityCooldownRemaining,AbilityCooldownDuration);
		if (AbilityCooldownRemaining>0.f)
		{
			StartCoolDown(AbilityCooldownDuration,AbilityCooldownRemaining);
		}
		float AbilityCost=InventoryItem->GetManaCost();
		
		ManaCostText->SetVisibility(AbilityCost==0.f ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		ManaCostText->SetText(FText::AsNumber(AbilityCost));

		CooldownDurationText->SetVisibility(AbilityCooldownDuration==0.f ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		CooldownCountText->SetText(FText::AsNumber(AbilityCooldownDuration));

		BindCanCastAbilityDelegate();
	}
	else
	{
		UpdateCanCast(true);
		ManaCostText->SetVisibility(ESlateVisibility::Hidden);
		CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
		CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	}
}

bool UInventoryItemWidget::IsEmpty() const
{
	return !InventoryItem || !InventoryItem->IsValid();
}

void UInventoryItemWidget::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber=NewSlotNumber;
}

void UInventoryItemWidget::UpdateStackCount()
{
	if (InventoryItem)
	{
		StackCountText->SetText(FText::AsNumber(InventoryItem->GetStackCount()));
	}
}

UTexture2D* UInventoryItemWidget::GetIconTexture() const
{
	if (InventoryItem && InventoryItem->GetShopItem())
	{
		return InventoryItem->GetShopItem()->GetIcon();
	}

	return nullptr;
}

FInventoryItemHandle UInventoryItemWidget::GetItemHandle() const
{
	if (!IsEmpty())
	{
		return InventoryItem->GetHandle();
	}
	return FInventoryItemHandle::InvalidHandle();
}

void UInventoryItemWidget::UpdateCanCast(bool bCanCast)
{
	GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CanCastDynamicMaterialParaName, bCanCast ?1 : 0);
}

void UInventoryItemWidget::BindCanCastAbilityDelegate()
{
	if (InventoryItem)
	{
		//利用委托将Item与GAS解耦
		const_cast<UInventoryItem*>(InventoryItem)->OnAbilityCanCastUpdated.AddUObject(this,&ThisClass::UpdateCanCast);
	}
}

void UInventoryItemWidget::UnBindCanCastAbilityDelegate()
{
	if (InventoryItem)
	{
		const_cast<UInventoryItem*>(InventoryItem)->OnAbilityCanCastUpdated.RemoveAll(this);
	}
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	//获取DragDropObject
	if (!IsEmpty() && DragDropOpClass)
	{
		UInventoryItemDragDropOp* DragDropOp=NewObject<UInventoryItemDragDropOp>(this,DragDropOpClass);
		if (DragDropOp)
		{
			DragDropOp->SetDraggedItem(this);
			OutOperation=DragDropOp;
		}
	}
	
}

bool UInventoryItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (UInventoryItemWidget* OtherWidget=Cast<UInventoryItemWidget>(InOperation->Payload))
	{
		if (OtherWidget && !OtherWidget->IsEmpty())
		{
			//交换ItemWidget逻辑
			OnInventoryItemDropped.Broadcast(this,OtherWidget);
			return true;
		}
	}
	return Super::NativeOnDrop(InGeometry,InDragDropEvent,InOperation);
}

void UInventoryItemWidget::StartCoolDown(float CooldownDuration, float TimeRemaining)
{
	CooldownTimeRemaining=TimeRemaining;
	CooldownTimeDuration=CooldownDuration;
	GetWorld()->GetTimerManager().SetTimer(CooldownDurationTimerHandle,this,&UInventoryItemWidget::CooldownFinished,CooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownUpdateTimerHandle,this,&ThisClass::UpdateCooldown,CooldownUpdateInterval,true);

	CooldownCountText->SetVisibility(ESlateVisibility::Visible);
}

void UInventoryItemWidget::CooldownFinished()
{
	GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimerHandle);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);

	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmtDynamicMaterialParaName,1.f);
	}
}

void UInventoryItemWidget::UpdateCooldown()
{
	CooldownTimeRemaining-=CooldownUpdateInterval;
	const float CooldownAmt=1.f-CooldownTimeRemaining/CooldownTimeDuration;
	CooldownDisplayFormattingOptions.MaximumFractionalDigits=CooldownTimeRemaining>1.f ? 0 : 2;
	CooldownCountText->SetText(FText::AsNumber(CooldownTimeRemaining,&CooldownDisplayFormattingOptions));
	
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmtDynamicMaterialParaName,CooldownAmt);
	}
}

void UInventoryItemWidget::ClearCooldown()
{
	CooldownFinished();
}

void UInventoryItemWidget::SetIcon(UTexture2D* IconTexture)
{
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetTextureParameterValue(IconTextureDynamicMaterialParaName,IconTexture);
		return ;
	}
	
	Super::SetIcon(IconTexture);
}
