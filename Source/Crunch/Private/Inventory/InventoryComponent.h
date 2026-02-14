// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


/*
 *	角色组件：处理ShopWidget相关的逻辑，处理Inventory和Shop之间的Item逻辑
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryItem.h"
#include "InventoryComponent.generated.h"

class UPA_ShopItem;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAddedDelegate, const UInventoryItem*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemoveDelegate, const FInventoryItemHandle&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemStackCountChangeDelegate, const FInventoryItemHandle&,int);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemAbilityCommitted, const FInventoryItemHandle&, float /*CooldownDuration*/, float /*CooldownTimeRemaining*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRUNCH_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnItemAddedDelegate OnItemAddedDelegate;
	FOnItemRemoveDelegate OnItemRemoveDelegate;
	FOnItemAbilityCommitted OnItemAbilityCommitted;
	FOnItemStackCountChangeDelegate OnItemStackCountChangeDelegate;
	
	//作为Input触发的回调，这个函数发生在客户端，利用RPC服务端尝试激活Item，应用GE或者赋予GA
	void TryActivateItem(const FInventoryItemHandle& ItemHandle);

	void SellItem(const FInventoryItemHandle& ItemHandle);
	
	// Sets default values for this component's properties
	UInventoryComponent();

	//监听委托的回调函数，判断完Item有效性后调用Server_Purchase
	void TryPurchase(const UPA_ShopItem* ItemToPurchase);

	//获取当前GoldAttribute值
	float GetGold() const;

	FORCEINLINE int GetCapacity() const { return Capacity; }
	
	void ItemSlotChange(const FInventoryItemHandle Handle,int NewSlotNumber);

	UInventoryItem* GetInventoryItemByHandle(const FInventoryItemHandle Handle) const ;

	//判断仓库是否已经存满（包括Slot容量判断在内），即不可以存放当前Item 
	bool IsFullFor(const UPA_ShopItem* Item) const ;

	//判断Slot是否都已经存在Item占用
	bool IsAllSlotOccupied() const;

	//遍历Map查询当前Item所属Slot，判断Stack是否还可以存储，如果可以返回InventoryItem,增加其Count
	UInventoryItem* GetAvailableStackForItem(const UPA_ShopItem* Item) const ;

	//判断当前PA对应的Item所需合成的所有子Item是否都可以在Inventory中被找到
	bool FindIngredientForItem(const UPA_ShopItem* Item,TArray<UInventoryItem*>& OutIngredients,const TArray<const UPA_ShopItem*>& IngredientToIgnore=TArray<const UPA_ShopItem*>()) const;

	//找到ShopItem资产对应的背包Item
	UInventoryItem* TryGetItemForShopItem(const UPA_ShopItem* Item) const ;

	//利用Slot槽位下标配合键盘对应数字键位输入触发IA，激活对应的Item
	void TryActivateItemInSlot(int SlotNumber);
	
protected:
	//获取ASC
	virtual void BeginPlay() override;

private:
	//仓库容量
	UPROPERTY(EditDefaultsOnly,Category="Inventory")
	int Capacity=6;
	
	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;

	//HandleID和Item的映射存储
	UPROPERTY()
	TMap<FInventoryItemHandle, UInventoryItem*> InventoryMap;

	//获取激活GA的信息，更新UI
	void AbilityCommitted(UGameplayAbility* CommittedAbility);

	/******************* Server **********************/

	//购买，出售逻辑应该在服务端执行，修改属性值，自动同步到客户端
	UFUNCTION(Server, Reliable,WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);

	UFUNCTION(Server, Reliable,WithValidation)
	void Server_ActivateItem(FInventoryItemHandle ItemHandle);

	UFUNCTION(Server, Reliable,WithValidation)
	void Server_SellItem(FInventoryItemHandle ItemHandle);

	//购买Item中调用，处理ItemCount和客户端UI的修改
	void GrantItem(const UPA_ShopItem* NewItem);

	//激活Item后如果Item是可消耗品则调用
	void ConsumeItem(UInventoryItem* Item);

	//RPC调用，在SellItem或者Consume使得Stack为0时调用，将其从Slot中移除，包含客户端调用逻辑
	void RemoveItem(UInventoryItem* Item);

	//判断当前生成的Item是否满足合成条件
	bool TryItemCombination(const UPA_ShopItem* NewItem);
	
private:
	//在客户端也生成一个与服务端相同的InventoryItem
	UFUNCTION(Client,Reliable)
	void Client_ItemAdded(FInventoryItemHandle AssignHandle,const UPA_ShopItem* Item,FGameplayAbilitySpecHandle GrantedAbilitySpecHandle);

	//当Item激活后消耗Stack，Stack为0时调用用于Remove
	UFUNCTION(Client,Reliable)
	void Client_ItemRemoved(FInventoryItemHandle ItemHandle);
		
	//在客户端也进行StackCount广播委托修改Widget的StackText值
	UFUNCTION(Client,Reliable)
	void Client_ItemStackCountChangeAdded(FInventoryItemHandle Handle,int NewCount);
		
};