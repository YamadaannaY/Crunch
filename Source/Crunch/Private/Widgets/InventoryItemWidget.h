// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemWidget.h"
#include "Inventory/InventoryItem.h"
#include "InventoryItemWidget.generated.h"

class UInventoryItem;
class UInventoryItemWidget;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemDropped,UInventoryItemWidget*,UInventoryItemWidget*)

/**
 * 
 */
UCLASS()
class CRUNCH_API UInventoryItemWidget : public UItemWidget
{
	GENERATED_BODY()
public:
	FOnInventoryItemDropped OnInventoryItemDropped;
	
	virtual void NativeConstruct() override;

	//在Inventory中传入Item，调用这个函数接收，进行文本赋值和Icon显示
	void UpdateInventoryItem(const UInventoryItem* Item);

	//对Slot进行清空操作，恢复空的状态
	void EmptySlot();

	//判断当前Slot是否为空
	bool IsEmpty() const;

	//对应的SlotNum
	FORCEINLINE int GetSlotNumber() const {return SlotNumber;}

	//更新SlotNumber
	void SetSlotNumber(int NewSlotNumber);

	//根据InventoryItem的Stack值更新StackText
	void UpdateStackCount();

	UTexture2D* GetIconTexture() const;

	FORCEINLINE const UInventoryItem* GetInventoryItem() const {return InventoryItem;}

	FInventoryItemHandle GetItemHandle() const;
private:

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

	//对应Slot的序号
	int SlotNumber;

	/********************** Drag Drop *******************/

private:
    //开始阶段，在DetectDrag并且移动鼠标进行拖拽时调用，创建Op对象并作为OutOp，创建后调用SetItem将此UI作为DragItem
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	//Drag结束时调用，在松开鼠标按键时对应的Widget位置上调用，用来实现交换Item的具体逻辑
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(EditDefaultsOnly,Category="DragDrop")
	TSubclassOf<class UInventoryItemDragDropOp> DragDropOpClass;
};
