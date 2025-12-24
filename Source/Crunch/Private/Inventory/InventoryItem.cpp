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

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem)
{
	Handle=NewHandle;
	ShopItem=NewShopItem;
}

void UInventoryItem::ApplyGasModifications(UAbilitySystemComponent* AbilitySystemComponent)
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