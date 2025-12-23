// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/ItemWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "ShopItemWidget.generated.h"

class UShopItemWidget;
class UPA_ShopItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemPurchaseIssused,const UPA_ShopItem*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopItemSelected,const UShopItemWidget*);

/**
 * 商店中的ItemUI
 */
UCLASS()
class UShopItemWidget : public UItemWidget ,public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	FOnItemPurchaseIssused OnItemPurchaseIssued;
	FOnShopItemSelected OnShopItemClicked;
	
	//ItemUI用ListView统一管理，调用OnSet保证Widget加入到ListView中立刻加载Icon
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
	FORCEINLINE const UPA_ShopItem* GetShopItem() const {return ShopItem;}
private:

	//在ItemWidget基类的Icon基础上添加PA作为Item信息
	UPROPERTY()
	const UPA_ShopItem* ShopItem;

	//Override

	//右键触发PurchaseIssued委托，广播一个ShopItem,其中存储了Price
	virtual void RightButtonClicked() override;

	//左键触发一个ShopItemSelected委托回调，广播自身ItemWidget，用于选择此Widget
	virtual void LeftButtonClicked() override;
};
