// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_ShopItem.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UPA_ShopItem;

//多个Item的集合结构体
USTRUCT(BlueprintType)
struct FItemCollection
{
	GENERATED_BODY()
public:
	FItemCollection();
	
	FItemCollection(const TArray<const UPA_ShopItem*>& InItems);

	//添加Item
	void AddItem(const UPA_ShopItem* NewItem,bool bAddUnique=false);
	
	//判断是否已经含有Item，结合bAddUnique使用
	bool Contains(const UPA_ShopItem* Item) const;
	
	//获取Items	
	const TArray<const UPA_ShopItem*>& GetItems() const;
	
private:
	//PAItem数组
	UPROPERTY()
	TArray<const UPA_ShopItem*> Items;
};

/**
 * 
 */

UCLASS()
class CRUNCH_API UPA_ShopItem : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	//将资产名（比如 DA_Sword）和类型（ShopItem）组合成唯一ID：ShopItem:DA_Sword
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

    //所有的UPA_ShopItem资产都属于ShopItem大类
	static FPrimaryAssetType GetShopItemAssetType();

	FText GetItemName() const {return ItemName;}
	FText GteItemDescription() const {return ItemDescription;}
	float GetPrice() const {return Price;}
	float GetSellPrice() const {return Price/2.f;}
	bool GetIsStackable() const {return bIsStackable;}
	bool GetIsConsumable() const {return bIsConsumable;}
	int GetMaxStackCount() const {return MaxStackCount;}
	TSubclassOf<UGameplayEffect> GetEquippedEffect() const {return EquippedEffect;}
	TSubclassOf<UGameplayEffect> GetConsumeEffect() const {return ConsumeEffect;}
	TSubclassOf<UGameplayAbility> GetGrantedAbility() const {return GrantAbility;}
	const TArray<TSoftObjectPtr<UPA_ShopItem>>& GetIngredients() const {return IngredientItems;}
	
	//加载Icon
	UTexture2D* GetIcon() const ;
	//获取GrantAbility的GetDefaultObject
	UGameplayAbility* GetGrantedAbilityCDO() const;
	
private:
	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly,category="ShopItem")
	float Price;

	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	FText ItemDescription;

	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	bool bIsConsumable;

	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	TSubclassOf<UGameplayEffect> ConsumeEffect;

	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	TSubclassOf<UGameplayEffect> EquippedEffect;

	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	TSubclassOf<UGameplayAbility> GrantAbility;

	//这个物品是否可以叠加存储
	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	bool bIsStackable=false;

	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	int MaxStackCount=5;
	
	UPROPERTY(EditDefaultsOnly,Category="ShopItem")
	TArray<TSoftObjectPtr<UPA_ShopItem>> IngredientItems;
};
