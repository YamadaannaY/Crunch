// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryItem.h"
#include "PA_ShopItem.h"
#include "GAS/UCAbilitySystemStatics.h"

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

UInventoryItem::UInventoryItem():StackCount(1),Slot(0),ShopItem(nullptr),OwnerASC(nullptr)
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

float  UInventoryItem::GetAbilityCooldownTimeRemaining() const
{
	if (!IsGrantingAnyAbility()) return 0.f;

	return UCAbilitySystemStatics::GetCoolDownRemainingFor(GetShopItem()->GetGrantedAbilityCDO(),*OwnerASC);
}

float UInventoryItem::GetAbilityCooldownDuration() const
{
	if (!IsGrantingAnyAbility()) return 0.f;

	return UCAbilitySystemStatics::GetCoolDownDurationFor(GetShopItem()->GetGrantedAbilityCDO(),*OwnerASC,1);
}

float UInventoryItem::GetManaCost() const
{
	if (!IsGrantingAnyAbility())
	{
		return 0.f;
	}

	return UCAbilitySystemStatics::GetManaCostFor(GetShopItem()->GetGrantedAbilityCDO(),*OwnerASC,1);
}

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem,UAbilitySystemComponent* ASC)
{
	Handle=NewHandle;
	ShopItem=NewShopItem;
	OwnerASC=ASC;
	ApplyGASModifications();
}

void UInventoryItem::ApplyGASModifications()
{
	if (!GetShopItem() || !OwnerASC) return ;
	
	if (!OwnerASC->GetOwner() || !OwnerASC->GetOwner()->HasAuthority()) return ;
	
	TSubclassOf<UGameplayEffect> EquipEffect=GetShopItem()->GetEquippedEffect();

	if (EquipEffect)
	{
		ApplyEquippedEffectHandle=OwnerASC->BP_ApplyGameplayEffectToSelf
		(EquipEffect,1,OwnerASC->MakeEffectContext());
	}

	TSubclassOf<UGameplayAbility> GrantedAbility=GetShopItem()->GetGrantedAbility();

	if (GrantedAbility)
	{
		GrantedAbilitySpecHandle=OwnerASC->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
	}
}

void UInventoryItem::RemoveGASModifications()
{
	if (!OwnerASC) return ;

	if (ApplyEquippedEffectHandle.IsValid())
	{
		OwnerASC->RemoveActiveGameplayEffect(ApplyEquippedEffectHandle);
	}
	if (GrantedAbilitySpecHandle.IsValid())
	{
		//这个方法会允许当前正在执行此GA时让这个GA执行完毕再删除
		OwnerASC->SetRemoveAbilityOnEnd(GrantedAbilitySpecHandle);
	}
}

bool UInventoryItem::IsGrantingAnyAbility() const 
{
	if (!ShopItem)
	{
		return false;
	}

	return ShopItem->GetGrantedAbility() !=nullptr;
}

void UInventoryItem::SetSlot(int NewSlot)
{
	Slot=NewSlot;
}

bool UInventoryItem::TryActivateGrantedAbility()
{
	if (!GrantedAbilitySpecHandle.IsValid()) return false;

	if (OwnerASC && OwnerASC->TryActivateAbility(GrantedAbilitySpecHandle))
	{
		return true;
	}
	return false;
}

void UInventoryItem::ApplyConsumeEffect()
{
	if (!ShopItem) return ;

	TSubclassOf<UGameplayEffect> ConsumeEffect=GetShopItem()->GetConsumeEffect();
	if (!ConsumeEffect) return ;

	OwnerASC->BP_ApplyGameplayEffectToSelf(ConsumeEffect,1,OwnerASC->MakeEffectContext());
}

bool UInventoryItem::IsValid() const
{
	return ShopItem != nullptr;
}
