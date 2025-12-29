// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PA_ShopItem.h"
#include "GAS/CHeroAttributeSet.h"


// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent() :OwnerASC(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

void UInventoryComponent::TryPurchase(const UPA_ShopItem* ItemToPurchase)
{
	if (!OwnerASC) return ;

	Server_Purchase(ItemToPurchase);
}

float UInventoryComponent::GetGold() const
{
	bool bFound=false;
	if (OwnerASC)
	{
		float Gold= OwnerASC->GetGameplayAttributeValue(UCHeroAttributeSet::GetGoldAttribute(), bFound);

		if (bFound) return Gold;
	}
	return 0.f;
}

void UInventoryComponent::ItemSlotChange(const FInventoryItemHandle Handle, int NewSlotNumber)
{
	if(UInventoryItem* FoundItem=GetInventoryItemByHandle(Handle))
	{
		FoundItem->SetSlot(NewSlotNumber);
	}
}

UInventoryItem* UInventoryComponent::GetInventoryItemByHandle(const FInventoryItemHandle Handle) const
{
	UInventoryItem* const* FoundItem=InventoryMap.Find(Handle);
	if (FoundItem)
	{
		return *FoundItem;
	}
	return nullptr;
}

bool UInventoryComponent::IsFullFor(const UPA_ShopItem* Item) const
{
	if (!Item) return false;

	if (IsAllSlotOccupied())
	{
		return GetAvailableStackForItem(Item) ==nullptr;
	}

	return false;
}

bool UInventoryComponent::IsAllSlotOccupied() const
{
	return InventoryMap.Num() >=GetCapacity();
}

UInventoryItem* UInventoryComponent::GetAvailableStackForItem(const UPA_ShopItem* Item) const
{
	if (!Item->GetIsStackable()) return nullptr;

	for (const TPair<FInventoryItemHandle,UInventoryItem*>& ItemPair: InventoryMap)
	{
		if (ItemPair.Value && ItemPair.Value->IsForItem(Item) && !ItemPair.Value->IsStackFull())
		{
			return ItemPair.Value;
		}
	}
	return nullptr;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}

void UInventoryComponent::GrantItem(const UPA_ShopItem* NewItem)
{
	if (!GetOwner()->HasAuthority()) return;
	if (UInventoryItem* StackItem=GetAvailableStackForItem(NewItem))
	{
		//服务端上的Item增加Count
		StackItem->AddStackCount();
		OnItemStackCountChangeDelegate.Broadcast(StackItem->GetHandle(),StackItem->GetStackCount());
		//客户端触发委托回调，在回调中修改客户端UI的StackText
		Client_ItemStackCountChangeAdded(StackItem->GetHandle(),StackItem->GetStackCount());
	}
	//占用Stack满或者没有这个Item，需要占用新Slot
	else
	{
		//创建InventoryItem,对应分配一个ID
		UInventoryItem* InventoryItem=NewObject<UInventoryItem>();
		FInventoryItemHandle NewHandle=FInventoryItemHandle::CreateHandle();

		InventoryItem->InitItem(NewHandle,NewItem);

		//存到哈希表中
		InventoryMap.Add(NewHandle,InventoryItem);

		//当库存Item创建成功后广播这个Item
		OnItemAddedDelegate.Broadcast(InventoryItem);

		UE_LOG(LogTemp,Warning,TEXT("Server Adding Shop Item:%s,with id :%d"),*(InventoryItem->GetShopItem()->GetItemName().ToString()),NewHandle.GetHandleId());

		//在客户端也进行相同的Item生成操作
		Client_ItemAdded(NewHandle,NewItem);

		//应用ItemGE
		InventoryItem->ApplyGasModifications(OwnerASC);
		
	}
}

void UInventoryComponent::Client_ItemStackCountChangeAdded_Implementation(FInventoryItemHandle Handle,
	int NewCount)
{
	if (GetOwner()->HasAuthority()) return;

	UInventoryItem* FoundItem=GetInventoryItemByHandle(Handle);
	if (FoundItem)
	{
		//修改Item值
		FoundItem->SetStackCount(NewCount);

		//调用广播吗，根据Item值修改Text
		OnItemStackCountChangeDelegate.Broadcast(Handle,NewCount);
	}
}

void UInventoryComponent::Client_ItemAdded_Implementation(FInventoryItemHandle AssignHandle, const UPA_ShopItem* Item)
{
	if (GetOwner()->HasAuthority()) return;

	UInventoryItem* InventoryItem=NewObject<UInventoryItem>();

	InventoryItem->InitItem(AssignHandle,Item);
	InventoryMap.Add(AssignHandle,InventoryItem);
	
	OnItemAddedDelegate.Broadcast(InventoryItem);

	UE_LOG(LogTemp,Warning,TEXT("Client Adding Shop Item:%s,with id :%d"),*(InventoryItem->GetShopItem()->GetItemName().ToString()),AssignHandle.GetHandleId());
}

bool UInventoryComponent::Server_Purchase_Validate(const UPA_ShopItem* ItemToPurchase)
{
	return true;
}

void UInventoryComponent::Server_Purchase_Implementation(const UPA_ShopItem* ItemToPurchase)
{
	if (!ItemToPurchase) return ;

	if (GetGold()<ItemToPurchase->GetPrice()) return ;
	if (IsFullFor(ItemToPurchase)) return ;

	//这个函数会触发PostGameplayEffectExecute,常用于逻辑修改，这里进行Add，修改Gold值
	OwnerASC->ApplyModToAttribute(UCHeroAttributeSet::GetGoldAttribute(),EGameplayModOp::Additive,-ItemToPurchase->GetPrice());

	UE_LOG(LogTemp,Warning,TEXT("Bought Item:%s"),*(ItemToPurchase->GetName()));

	GrantItem(ItemToPurchase);
}