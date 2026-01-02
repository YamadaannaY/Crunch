// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PA_ShopItem.h"
#include "Framework/CAssetManager.h"
#include "GAS/CHeroAttributeSet.h"


void UInventoryComponent::TryActivateItem(const FInventoryItemHandle& ItemHandle)
{
	UInventoryItem* InventoryItem=GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem) return ;

	//服务端应用Item
	Server_ActivateItem(ItemHandle);
}

void UInventoryComponent::SellItem(const FInventoryItemHandle& ItemHandle)
{
	Server_SellItem(ItemHandle);
}

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

bool UInventoryComponent::FoundIngredientForItem(const UPA_ShopItem* Item,
	TArray<UInventoryItem*>& OutIngredients) const
{
	//获取合成需要的Item
	const FItemCollection* Ingredients=UCAssetManager::Get().GetIngredientForItem(Item);
	if (!Ingredients)
	{
		return false;
	}
	bool bAllFound=true;
	for (const UPA_ShopItem* Ingredient : Ingredients->GetItems())
	{
		//遍历获取InventoryItem
		UInventoryItem* FoundItem=TryGetItemForShopItem(Ingredient);
		if (!FoundItem)
		{
			bAllFound=false;
			break;
		}
		//全部找到后进行存储
		OutIngredients.Add(FoundItem);
	}
	return bAllFound;
}

UInventoryItem* UInventoryComponent::TryGetItemForShopItem(const UPA_ShopItem* Item) const
{
	if (!Item) return nullptr;

	//遍历InventoryMap，利用PA找到对应的InventoryItem
	for (const TPair<FInventoryItemHandle,UInventoryItem*>& ItemHandlePair: InventoryMap)
	{
		if (ItemHandlePair.Value && ItemHandlePair.Value->GetShopItem()==Item)
		{
			return ItemHandlePair.Value;
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

void UInventoryComponent::Server_ActivateItem_Implementation(FInventoryItemHandle ItemHandle)
{
	//获取Item
	UInventoryItem* InventoryItem=GetInventoryItemByHandle(ItemHandle);
	if (!InventoryItem) return ;

	//如果有ASC中有GA，尝试执行。     获取ShopItem
	InventoryItem->TryActivateGrantedAbility(OwnerASC);
	const UPA_ShopItem* Item=InventoryItem->GetShopItem();

	//判断是否是可以Consume的Item，即有主动消耗的效果
	if (Item->GetIsConsumable())
	{
		ConsumeItem(InventoryItem);
	}
}

bool UInventoryComponent::Server_ActivateItem_Validate(FInventoryItemHandle ItemHandle)
{
	return true;
}

void UInventoryComponent::Server_SellItem_Implementation(FInventoryItemHandle ItemHandle)
{
	UInventoryItem* InventoryItem=GetInventoryItemByHandle(ItemHandle);

	if (!InventoryItem || !InventoryItem->IsValid()) return ;

	const float SellPrice=InventoryItem->GetShopItem()->GetSellPrice();
	//修改属性值
	OwnerASC->ApplyModToAttribute(UCHeroAttributeSet::GetGoldAttribute(),EGameplayModOp::Additive,SellPrice*InventoryItem->GetStackCount());

	//这里执行的逻辑是全部卖掉
	RemoveItem(InventoryItem);
}

bool UInventoryComponent::Server_SellItem_Validate(FInventoryItemHandle ItemHandle)
{
	return true;
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
		InventoryItem->ApplyGASModifications(OwnerASC);

		//显然合成只有在新Slot被占据的时候才会发生，进行检查
		CheckItemCombination(InventoryItem);
	}
}

void UInventoryComponent::ConsumeItem(UInventoryItem* Item)
{
	if (!GetOwner()->HasAuthority()) return;
	if (!Item) return;

	//调用GE
	Item->ApplyConsumeEffect(OwnerASC);

	//自减一次，如果此时StackCount<0,将Item移除
	if (!Item->ReduceStackCount())
	{
		RemoveItem(Item);
	}
	else
	{
		//更新自减后的ItemStack
		OnItemStackCountChangeDelegate.Broadcast(Item->GetHandle(),Item->GetStackCount());
		Client_ItemStackCountChangeAdded(Item->GetHandle(),Item->GetStackCount());
	}
}

void UInventoryComponent::RemoveItem(UInventoryItem* Item)
{
	if (!GetOwner()->HasAuthority()) return ;
	
	Item->RemoveGASModifications(OwnerASC);
	OnItemRemoveDelegate.Broadcast(Item->GetHandle());
	InventoryMap.Remove(Item->GetHandle());
	Client_ItemRemoved(Item->GetHandle());
}

void UInventoryComponent::CheckItemCombination(const UInventoryItem* NewItem)
{
	if (!GetOwner()->HasAuthority()) return ;

	//找到新生成Item可以合成的所有Item集合
	const FItemCollection* CombinationItems=UCAssetManager::Get().GetCombinationForItem(NewItem->GetShopItem());
	if (!CombinationItems) return ;

	//遍历可合成Item的PA
	for (const UPA_ShopItem* CombinationItem:CombinationItems->GetItems())
	{
		TArray<UInventoryItem*> Ingredients;

		//判断是否可以合成
		if (!FoundIngredientForItem(CombinationItem,Ingredients)) continue;

		//说明可以合成，所有子Item都已经在Inventory中被找到并且存储在了Ingredients中
		for (UInventoryItem* Ingredient : Ingredients)
		{
			RemoveItem(Ingredient);
		}
		GrantItem(CombinationItem);
		return ;
	}
}

void UInventoryComponent::Client_ItemRemoved_Implementation(FInventoryItemHandle ItemHandle)
{
	if (GetOwner()->HasAuthority()) return;

	UInventoryItem* InventoryItem= GetInventoryItemByHandle(ItemHandle);

	if (!InventoryItem) return ;

	//不用处理ASC相关
	OnItemRemoveDelegate.Broadcast(ItemHandle);
	InventoryMap.Remove(ItemHandle);
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