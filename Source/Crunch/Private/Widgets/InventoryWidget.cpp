// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include  "Components/WrapBox.h"
#include  "Components/WrapBoxSlot.h"
#include "Inventory/InventoryComponent.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (APawn* OwnerPawn=GetOwningPlayerPawn())
	{
		InventoryComponent=OwnerPawn->GetComponentByClass<UInventoryComponent>();
		if (InventoryComponent)
		{
			//在Component中准备好的委托，购买Item后调用的GrantItem中广播，执行对Item的操作
			InventoryComponent->OnItemAddedDelegate.AddUObject(this,&ThisClass::ItemAdded);

			const int Capacity = InventoryComponent->GetCapacity();

			//初始化
			ItemList->ClearChildren();

			//遍历仓库容量次数
			for (int i=0;i<Capacity;i++)
			{
				//创建ItemWidget，为Widget分配SlotNumber，添加到WrapBox中
				UInventoryItemWidget* NewEmptyWidget=CreateWidget<UInventoryItemWidget>(GetOwningPlayer(),ItemWidgetClass);
				if (NewEmptyWidget)
				{
					NewEmptyWidget->SetSlotNumber(i);
					UWrapBoxSlot* NewItemSlot=ItemList->AddChildToWrapBox(NewEmptyWidget);
					NewItemSlot->SetPadding(FMargin(2.f));

					//Item成功添加到Inventory，添加到ItemWidgets数组中存储
					ItemWidgets.Add(NewEmptyWidget);
				}
			}
		}
	}
}

void UInventoryWidget::ItemAdded(const UInventoryItem* InventoryItem)
{
	if (!InventoryItem) return;

	if (UInventoryItemWidget* NextAvailableSlot=GetNextAvailableSlot())
	{
		//根据Item更新Slot
		NextAvailableSlot->UpdateInventoryItem(InventoryItem);

		//将Slot和Item以Handle关联
		PopulatedItemEntryWidgets.Add(InventoryItem->GetHandle(),NextAvailableSlot);
		if (InventoryComponent)
		{
			//将SlotNumber传递给Item更新
			InventoryComponent->ItemSlotChange(InventoryItem->GetHandle(),NextAvailableSlot->GetSlotNumber());
		}
	}
}

UInventoryItemWidget* UInventoryWidget::GetNextAvailableSlot() const
{
	for (UInventoryItemWidget* Widget : ItemWidgets)
	{
		if (Widget->IsEmpty())
		{
			return Widget;
		}
	}

	return nullptr;
}
