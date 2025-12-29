// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemWidget.h"
#include "Inventory/InventoryItem.h"
#include "InventoryItemWidget.generated.h"

class UInventoryItem;
/**
 * 
 */
UCLASS()
class CRUNCH_API UInventoryItemWidget : public UItemWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	//在Inventory中传入Item，调用这个函数接收，进行文本赋值和Icon显示
	void UpdateInventoryItem(const UInventoryItem* Item);

	//对Slot进行清空操作，恢复空的状态
	void EmptySlot();

	//判断当前Slot是否为空
	bool IsEmpty() const;

	//对应的SlotNum
	FORCEINLINE int GetSlotNumber() const {return SlotNumber;}

	void SetSlotNumber(int NewSlotNumber);
private:
	void UpdateStackCount();

	//格子为空时显示的Texture
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	UTexture2D* EmptyTexture;

	//Item的叠加数量
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* StackCountText;

	//Item的计数冷却时间
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CooldownCountText;

	//Item的持续时间
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CooldownDurationText;

	//使用Item需要的Mana消耗
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ManaCostText;

	//已经创建好的具有ItemHandle的InventoryItem
	UPROPERTY()
	const UInventoryItem* InventoryItem;

	int SlotNumber;
};
