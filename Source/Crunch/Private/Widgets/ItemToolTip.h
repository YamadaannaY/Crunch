// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/PA_ShopItem.h"
#include "ItemToolTip.generated.h"

class UPA_ShopItem;
/**
 * 
 */
UCLASS()
class UItemToolTip : public UUserWidget
{
	GENERATED_BODY()

public:
	//设置ToolTip对应的Item
	void SetItem(const UPA_ShopItem* Item);

	//改变Item的Price值
	void SetPrice(float NewPrice);
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* IconImage;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ItemTitleText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemDescriptionText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemPriceText;
};
