// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CAbilitySystemComponent.h"
#include "UObject/Object.h"
#include "GameplayEffectTypes.h"
#include "PA_ShopItem.h"
#include "InventoryItem.generated.h"

/*
 *与其只在服务端构建Item然后发送给客户端，不如直接在两端分别创建，然后以一个Handle进行管理，每个Handle分配一个独特的ID
 *当Purchase在服务端进行完毕后，服务端向客户端发送这个ID，客户端根据ID找到Handle对应的Item进行相同的操作
 *对应关系：PA_ShopItem+ItemHandle=InventoryItem
 */


class UPA_ShopItem;

USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()

public:
	FInventoryItemHandle();
	
	//利用有参构造函数调用ID生成函数创建一个Handle
	static FInventoryItemHandle CreateHandle();
	
	static FInventoryItemHandle InvalidHandle();
	bool IsValid() const;
	uint32 GetHandleId() const;
	
private:
	//对于传入int值的ID，需要防止隐式类型转换（即一个int值被参数类型为Handle的函数判定为隐式构造）
	explicit FInventoryItemHandle(uint32 ID);

	//Handle唯一的ID
	UPROPERTY()
	uint32 HandleId;

	//静态值递增实现
	static uint32 GeneratedNextID();

	//定义InvalidID为0
	static uint32 GetInvalidId();
};

inline bool operator==(const FInventoryItemHandle& Lhs,const FInventoryItemHandle& Rhs)
{
	return Lhs.GetHandleId()==Rhs.GetHandleId();
}

//获取Handle的ID
uint32 GetTypeHash(const FInventoryItemHandle& Key);

/**
 * 
 */

UCLASS()
class CRUNCH_API UInventoryItem : public UObject
{
	GENERATED_BODY()
public:
	UInventoryItem();

	//增加StackCount,成功添加返回true
	bool AddStackCount();

	//return true if StackCount>0
	bool ReduceStackCount();

	bool SetStackCount(int NewStackCount);
	
	bool IsStackFull() const ;
	bool IsForItem(const UPA_ShopItem* Item) const; 
	
	//为新创建的InventoryItem赋值
	void InitItem(const FInventoryItemHandle& NewHandle,const UPA_ShopItem* NewShopItem);

	FORCEINLINE const UPA_ShopItem* GetShopItem() const {return ShopItem;}
	FORCEINLINE FInventoryItemHandle GetHandle() const {return Handle;}
	FORCEINLINE int GetStackCount() const {return StackCount;}

	//GrantedAbility激活
	bool TryActivateGrantedAbility(UAbilitySystemComponent* AbilitySystemComponent);

	//ConsumeEffect应用
	void ApplyConsumeEffect(UAbilitySystemComponent* AbilitySystemComponent);
	
	bool IsValid() const;

	//为ASC应用GE
	void ApplyGASModifications(UAbilitySystemComponent* AbilitySystemComponent);
	void RemoveGASModifications(UAbilitySystemComponent* AbilitySystemComponent);

	//更新Slot
	void SetSlot(int NewSlot);
private:
	FInventoryItemHandle Handle;

	UPROPERTY()
	const UPA_ShopItem* ShopItem;

	//已经叠加的层数
	int StackCount;

	//占的Slot位置
	int Slot;
	
	//为了移除Effect保留其Handle
	FActiveGameplayEffectHandle ApplyEquippedEffectHandle;

	//如果这个Item为角色赋予了GA，则为这个GA创建Handle进行保留
	FGameplayAbilitySpecHandle GrantedAbilitySpecHandle;
};
