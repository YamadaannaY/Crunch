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
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemStackCountChangeDelegate, const FInventoryItemHandle&,int);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRUNCH_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnItemAddedDelegate OnItemAddedDelegate;
	FOnItemStackCountChangeDelegate OnItemStackCountChangeDelegate;
	// Sets default values for this component's properties
	UInventoryComponent();

	//监听委托的回调函数，判断完Item有效性后调用Server_Purchase
	void TryPurchase(const UPA_ShopItem* ItemToPurchase);

	//获取GoldAttribute值
	float GetGold() const;

	FORCEINLINE int GetCapacity() const { return Capacity; }
	
	void ItemSlotChange(const FInventoryItemHandle Handle,int NewSlotNumber);

	UInventoryItem* GetInventoryItemByHandle(const FInventoryItemHandle Handle) const ;

	//调用判断仓库是否已经存满，即不可以存放当前Item 
	bool IsFullFor(const UPA_ShopItem* Item) const ;

	//判断生成的Item数量是否已经大于仓库容量
	bool IsAllSlotOccupied() const;

	//遍历Map查询当前Item所属Slot，判断Stack是否还可以存储，如果可以返回InventoryItem,增加其Count
	UInventoryItem* GetAvailableStackForItem(const UPA_ShopItem* Item) const ;

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

	/******************* Server **********************/

	//购买逻辑应该在服务端执行
	UFUNCTION(Server, Reliable,WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);

	//服务端购买操作实现后调用,为买到的ShopItem建立InventoryItem以及对应的ItemHandle,存储在Map中
	void GrantItem(const UPA_ShopItem* NewItem);

private:
	//在客户端也生成一个与服务端相同的InventoryItem
	UFUNCTION(Client,Reliable)
	void Client_ItemAdded(FInventoryItemHandle AssignHandle,const UPA_ShopItem* Item);

	//在客户端也进行StackCount广播委托修改Widget的StackText值
	UFUNCTION(Client,Reliable)
	void Client_ItemStackCountChangeAdded(FInventoryItemHandle Handle,int NewCount);
		
};
