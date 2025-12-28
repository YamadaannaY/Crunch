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
	
	//当ListView由空位要显示Widget时，会从对象池取出一个Widget（如果没有会调用CreateWidget）进行显示，此时调用这个函数
	//也就是要显示的Widget要调用的函数，适合用于初始化。
	//在本UI框架中，ShopWidget从AssMan中取出了所有PA并AddItem到了ListView，等价于ListItemObject
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
