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

	//遍历找到第一个空Slot
	UInventoryItemWidget* GetNextAvailableSlot() const ;

	//处理两个交换的ItemWidget的各种信息交换
	void HandleItemDragDrop(UInventoryItemWidget* DestinationWidget,UInventoryItemWidget* SourceWidget);

	//清空当前Item对应Slot，从数组中移除Handle
	void ItemRemoved(const FInventoryItemHandle& ItemHandle);

	//当Item对应GA被调用时，激活Widget的冷却逻辑
	void ItemAbilityCommitted(const FInventoryItemHandle& ItemHandle, float CooldownDuration, float CooldownTimeRemaining);
};
