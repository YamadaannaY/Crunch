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
	//从AssetManager中加载出所有PA_ShopItem回调ShopItemLoadFinished
	void LoadShopItem();

	//缓存所有ShopItems，遍历并添加到ShopList的数据池中
	void ShopItemLoadFinished();

	//绑定ItemWidget中的委托回调，处理购买，点击逻辑
	void ShopItemWidgetGenerated(UUserWidget& NewWidget);

	//点击时的回调，展示当前Item的CombinationTree
	void ShowItemCombination(const UShopItemWidget* ItemWidget);

	//管理所有UI的ListView
	UPROPERTY(meta=(BindWidget))
	UTileView* ShopItemList;

	//合成路线树
	UPROPERTY(meta=(BindWidget))
	UItemTreeWidget* CombinationTree;
	
	//存储所有ShopItemWidget
	UPROPERTY()
	TMap<const UPA_ShopItem*,const UShopItemWidget*> ItemsMap;

	//利用组件处理Purchase逻辑
	UPROPERTY()
	UInventoryComponent* InventoryComponent;

};