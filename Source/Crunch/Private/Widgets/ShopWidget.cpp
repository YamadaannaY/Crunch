// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ShopWidget.h"
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

	//创建一个新实例Widget时触发的委托回调，进行初始化，这个初始化是对于这个格子而言，数据更新不会调用此初始化
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
		//将这个PA加入Item数组中，作为Widget的数据。实际上的EntryWidget是一个简单的只有空Image的Widget，其显示由PA决定
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
		
		ItemsMap.Add(ItemWidget->GetShopItem(),ItemWidget);
	}
}

