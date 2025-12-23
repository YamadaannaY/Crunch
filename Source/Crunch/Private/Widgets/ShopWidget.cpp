// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ShopWidget.h"
#include "ShopItemWidget.h"
#include "Framework/CAssetManager.h"
#include "Components/TileView.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	//调用ShopItemType的加载
	LoadShopItem();

	//创建一个新实例Widget时触发，由于池化机制，这个函数不会被频繁调用，适合用于初始哈
	ShopItemList->OnEntryWidgetGenerated().AddUObject(this,&ThisClass::ShopItemWidgetGenerated);
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
		ShopItemList->AddItem(const_cast<UPA_ShopItem*>(ShopItem));		
	}
}


void UShopWidget::ShopItemWidgetGenerated(UUserWidget& NewWidget)
{
	//Cast到Item层，将其添加到Map中存储
	UShopItemWidget* ItemWidget=Cast<UShopItemWidget>(&NewWidget);

	if (ItemWidget)
	{
		ItemsMap.Add(ItemWidget->GetShopItem(),ItemWidget);
	}
}

