// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShopItemWidget.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryComponent.h"
#include "ShopWidget.generated.h"

class UItemTreeWidget;
class UShopItemWidget;
class UPA_ShopItem;
class UTileView;

/**
 * 商店UI
 */

UCLASS()
class UShopWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
private:
	//管理所有UI的ListView
	UPROPERTY(meta=(BindWidget))
	UTileView* ShopItemList;

	UPROPERTY(meta=(BindWidget))
	UItemTreeWidget* CombinationTree;

	void LoadShopItem();
	void ShopItemLoadFinished();
	void ShopItemWidgetGenerated(UUserWidget& NewWidget);

	//存储所有ShopItemWidget
	UPROPERTY()
	TMap<const UPA_ShopItem*,const UShopItemWidget*> ItemsMap;

	//利用组件处理Purchase逻辑
	UPROPERTY()
	UInventoryComponent* InventoryComponent;

	void ShowItemCombination(const UShopItemWidget* ItemWidget);
};