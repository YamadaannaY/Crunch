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

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityCanCastUpdateDelegate,bool)

USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()

	FInventoryItemHandle();
	
	//利用有参构造函数调用ID生成函数创建一个Handle
	static FInventoryItemHandle CreateHandle();

	//提供一个InvalidHandle
	static FInventoryItemHandle InvalidHandle();
	
	//判断是否等于定义的不合法ID
	bool IsValid() const;
	
	//封装ID
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
class UInventoryItem : public UObject
{
	GENERATED_BODY()
public:
	//更新Widget
	FOnAbilityCanCastUpdateDelegate OnAbilityCanCastUpdated;

	UInventoryItem();
	
	//增加StackCount,成功添加返回true
	bool AddStackCount();

	//return true if StackCount>0
	bool ReduceStackCount();

	//修改StackCount为NewCount
	bool SetStackCount(int NewStackCount);
	
	//判断当前Item的StackCount是否达到MaxCount（5）
	bool IsStackFull() const ;
	
	//判断传参ShopItem是否等于当前InventoryItem对应的ShopItem
	bool IsForItem(const UPA_ShopItem* Item) const;
	
	//为新创建的InventoryItem赋值
	void InitItem(const FInventoryItemHandle& NewHandle,const UPA_ShopItem* NewShopItem,UAbilitySystemComponent* ASC);

	//获取剩余冷却时间
	float GetAbilityCooldownTimeRemaining() const;
	
	//获取冷却持续时间
	float GetAbilityCooldownDuration() const ;
	
	//获取Mana消耗
	float GetManaCost() const ;

	const UPA_ShopItem* GetShopItem() const {return ShopItem;}
	FInventoryItemHandle GetHandle() const {return Handle;}
	int GetStackCount() const {return StackCount;}
	int GetItemSlot() const {return Slot;}
	FGameplayAbilitySpecHandle GetGrantedAbilitySpecHandle() const {return GrantedAbilitySpecHandle;}

	//判断是否可以释放GA
	bool CanCastAbility() const ;

	//GrantedAbility激活的服务端RPC函数上调用
	bool TryActivateGrantedAbility();

	//ConsumeEffect应用
	void ApplyConsumeEffect();
	
	//ShopItem指针是否有效
	bool IsValid() const;

	//为ASC应用GE
	void RemoveGASModifications();

	//是否具有GA
	bool IsGrantingAnyAbility() const ;
	
	//判断参数GA是否就是ItemGA
	bool IsGrantingAbility(TSubclassOf<UGameplayAbility> AbilityClass) const;

	//更新Slot
	void SetSlot(int NewSlot);
	
	//保存GA句柄用于删除
	void SetGrantedAbilitySpecHandle(FGameplayAbilitySpecHandle NewHandle);
private:
	//在Item生成并进行初始化操作时应用GE或注册GA
	void ApplyGASModifications();

	void ManaUpdated(const FOnAttributeChangeData& ChangeData) const ;

	//Item句柄
	FInventoryItemHandle Handle;

	//PA数据资产
	UPROPERTY()
	const UPA_ShopItem* ShopItem;

	/***** 仓库Item独有的参数		*****/
	//已经叠加的层数
	int StackCount;

	//占的Slot位置
	int Slot;

	//****       GA      *************//
	
	//为了移除Effect保留其Handle
	FActiveGameplayEffectHandle ApplyEquippedEffectHandle;

	//为了移除GA保留其Handle
	FGameplayAbilitySpecHandle GrantedAbilitySpecHandle;

	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;
};
