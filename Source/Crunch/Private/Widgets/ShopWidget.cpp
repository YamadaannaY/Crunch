// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ShopWidget.h"

#include "ItemTreeWidget.h"
#include "ShopItemWidget.h"
#include "Framework/CAssetManager.h"
#include "Components/TileView.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//打开Shop后聚焦
	SetIsFocusable(true);

	//调用ShopItemType的加载
	LoadShopItem();

	//在ShopItemWidget生成时调用的逻辑
	ShopItemList->OnEntryWidgetGenerated().AddUObject(this,&ThisClass::ShopItemWidgetGenerated);

	//获取组件
	if (APawn* OwnerPawn=GetOwningPlayerPawn())
	{
		InventoryComponent=OwnerPawn->GetComponentByClass<UInventoryComponent>();
	}
}

void UShopWidget::LoadShopItem()
{
	UCAssetManager::Get().LoadShopItems(FStreamableDelegate::CreateUObject(this,&ThisClass::ShopItemLoadFinished));
}

void UShopWidget::ShopItemLoadFinished()
{
	TArray<const UPA_ShopItem*> ShopItems;
	UCAssetManager::Get().GetLoadedShopItem(ShopItems);

	for (const UPA_ShopItem* ShopItem : ShopItems)
	{
		//添加到数据池中，由于NumEntries设置的足够大，被Add即立刻加载
		ShopItemList->AddItem(const_cast<UPA_ShopItem*>(ShopItem));		
	}
}

void UShopWidget::ShopItemWidgetGenerated(UUserWidget& NewWidget)
{
	//Cast到Item层，将其添加到Map中存储
	UShopItemWidget* ItemWidget=Cast<UShopItemWidget>(&NewWidget);
	
	if (ItemWidget)
	{
		//当生成实例时，为Purchase委托绑定回调
		if (InventoryComponent)
		{
			ItemWidget->OnItemPurchaseIssued.AddUObject(InventoryComponent,&UInventoryComponent::TryPurchase);
		}
		
		ItemWidget->OnShopItemClicked.AddUObject(this,&ThisClass::ShowItemCombination);
		ItemsMap.Add(ItemWidget->GetShopItem(),ItemWidget);
	}
}

void UShopWidget::ShowItemCombination(const UShopItemWidget* ItemWidget)
{
	if (CombinationTree)
	{
		CombinationTree->DrawFromNode(ItemWidget);
	}
}

