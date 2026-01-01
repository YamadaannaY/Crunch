// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryItem.h"
#include "PA_ShopItem.h"

FInventoryItemHandle::FInventoryItemHandle() :HandleId(GetInvalidId())
{
}

FInventoryItemHandle FInventoryItemHandle::InvalidHandle()
{
	static FInventoryItemHandle InvalidHandle = FInventoryItemHandle();

	return InvalidHandle;
}

FInventoryItemHandle FInventoryItemHandle::CreateHandle()
{
	return FInventoryItemHandle(GeneratedNextID());
}

bool FInventoryItemHandle::IsValid() const
{
	return HandleId!=GetInvalidId();
}

uint32 FInventoryItemHandle::GetHandleId() const
{
	return HandleId;
}

FInventoryItemHandle::FInventoryItemHandle(uint32 ID) :HandleId(ID)
{
	
}

uint32 FInventoryItemHandle::GeneratedNextID()
{
	static uint32 StaticID=1;
	return StaticID++;
}

uint32 FInventoryItemHandle::GetInvalidId()
{
	return 0;
}

uint32 GetTypeHash(const FInventoryItemHandle& Key)
{
	return Key.GetHandleId();
}

UInventoryItem::UInventoryItem():StackCount(1),Slot(0),ShopItem(nullptr)
{
	
}

bool UInventoryItem::AddStackCount()
{
	if (IsStackFull()) return false ;
	++StackCount;
	return true;
}

bool UInventoryItem::ReduceStackCount()
{
	--StackCount;
	if (StackCount<=0) return false;
	return true;
}

bool UInventoryItem::SetStackCount(int NewStackCount)
{
	if (NewStackCount>0 && NewStackCount<=GetShopItem()->GetMaxStackCount())
	{
		StackCount=NewStackCount;
		return true;
	}
	return false;
}

bool UInventoryItem::IsStackFull() const
{
	return StackCount>=GetShopItem()->GetMaxStackCount();
}

bool UInventoryItem::IsForItem(const UPA_ShopItem* Item) const
{
	if (!Item) return false;

	return GetShopItem()==Item;
}

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem)
{
	Handle=NewHandle;
	ShopItem=NewShopItem;
}

void UInventoryItem::ApplyGASModifications(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!GetShopItem() || !AbilitySystemComponent) return ;

	if (!AbilitySystemComponent->GetOwner() || !AbilitySystemComponent->GetOwner()->HasAuthority()) return ;
	
	TSubclassOf<UGameplayEffect> EquipEffect=GetShopItem()->GetEquippedEffect();

	if (EquipEffect)
	{
		ApplyEquippedEffectHandle=AbilitySystemComponent->BP_ApplyGameplayEffectToSelf
		(EquipEffect,1,AbilitySystemComponent->MakeEffectContext());
	}

	TSubclassOf<UGameplayAbility> GrantedAbility=GetShopItem()->GetGrantedAbility();

	if (GrantedAbility)
	{
		const FGameplayAbilitySpec* FoundSpec=AbilitySystemComponent->FindAbilitySpecFromClass(GrantedAbility);

		if (FoundSpec)
		{
			GrantedAbilitySpecHandle=FoundSpec->Handle;
		}
		else
		{
			GrantedAbilitySpecHandle=AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
		}
	}
}

void UInventoryItem::RemoveGASModifications(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent) return ;

	if (ApplyEquippedEffectHandle.IsValid())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(ApplyEquippedEffectHandle);
	}
	if (GrantedAbilitySpecHandle.IsValid())
	{
		//这个方法会允许当前正在执行此GA时让这个GA执行完毕再删除
		AbilitySystemComponent->SetRemoveAbilityOnEnd(GrantedAbilitySpecHandle);
	}
}

void UInventoryItem::SetSlot(int NewSlot)
{
	Slot=NewSlot;
}

bool UInventoryItem::TryActivateGrantedAbility(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!GrantedAbilitySpecHandle.IsValid()) return false;

	if (AbilitySystemComponent && AbilitySystemComponent->TryActivateAbility(GrantedAbilitySpecHandle))
	{
		return true;
	}
	return false;
}

void UInventoryItem::ApplyConsumeEffect(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!ShopItem) return ;

	TSubclassOf<UGameplayEffect> ConsumeEffect=GetShopItem()->GetConsumeEffect();
	if (!ConsumeEffect) return ;

	AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(ConsumeEffect,1,AbilitySystemComponent->MakeEffectContext());
}

bool UInventoryItem::IsValid() const
{
	return ShopItem != nullptr;
}
