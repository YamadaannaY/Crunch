 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemWidget.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

 class UInventoryContextMenu;

/**
 * 
 */
UCLASS()
class CRUNCH_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(meta=(BindWidget))
	class UWrapBox* ItemList;

	UPROPERTY(EditDefaultsOnly,Category="Inventory")
	TSubclassOf<UInventoryItemWidget> ItemWidgetClass;

	UPROPERTY()
	class UInventoryComponent* InventoryComponent;

	UPROPERTY()
	TArray<UInventoryItemWidget*> ItemWidgets;

	UPROPERTY()
	TMap<FInventoryItemHandle,UInventoryItemWidget*> PopulatedItemEntryWidgets;

	UPROPERTY(EditDefaultsOnly,Category="Inventory")
	TSubclassOf<UInventoryContextMenu> ContextMenuWidgetClass;

	UPROPERTY()
	UInventoryContextMenu* ContextMenuWidget;

	//创建MenuWidget组件
	void SpawnContextMenu();

	//售卖Item
	UFUNCTION()
	void SellFocusedItem();

	//与左键点击作用相同，使用Item
	UFUNCTION()
	void UseFocusedItem();

	//设置可见性
	void SetContextMenuVisible(bool bContextMenuVisible);

	//右键点击绑定的回调。切换Menu，如果已经点击过再点击->取消 如果点击另一个->切换，如果第一次点击->显示
	void ToggleContextMenu(const FInventoryItemHandle& ItemHandle);

	//重置Menu
	void ClearContextMenu();

	FInventoryItemHandle CurrentFocusedItemHandle;
	
	//当一个Item被购买后Grant时广播调用的委托，将Item作参数更新Slot
	void ItemAdded(const UInventoryItem* InventoryItem);

	//当一个Item可以StackCount时广播委托的回调函数
	void ItemStackCountChanged(const FInventoryItemHandle& Handle , int NewCount);

	UInventoryItemWidget* GetNextAvailableSlot() const ;

	void HandleItemDragDrop(UInventoryItemWidget* DestinationWidget,UInventoryItemWidget* SourceWidget);

	void ItemRemoved(const FInventoryItemHandle& ItemHandle);
};
