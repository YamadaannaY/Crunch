// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/PA_ShopItem.h"
#include "Engine/AssetManager.h"
#include "CAssetManager.generated.h"

class UPA_CharacterDefination;
/**
 * 
 */
UCLASS()
class CRUNCH_API UCAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	//返回全局单例的AssetManager并将其Cast到当前Manager类
	static UCAssetManager& Get();

	//一次性加载属于特定Type的所有资源，这里用于加载ShopItem,绑定一个回调函数，这个函数本身对所有加载出来的PA建立Ingredient，Combination映射,
	//额外带有一个跟随触发的委托，在所有此Asset加载完后广播，在ShopWidget中响应处理对所有ShopItem的操作
	void LoadShopItems(const FStreamableDelegate& LoadFinishedCallback);

	//加载CharacterDefPA的资源
	void LoadCharacterDefinition(const FStreamableDelegate& LoadFinishedCallback);

	//获得当前商店内所有被加载的Item
	bool GetLoadedShopItem(TArray<const UPA_ShopItem*>& OutItems) const ;

	//获得当前所有加载的CharacterDef
	bool GetLoadedCharacterDefinition(TArray<UPA_CharacterDefination*>& LoadedCharacterDefinition) const ;

	//获取Item可以合成的所有Item
	const FItemCollection* GetCombinationForItem(const UPA_ShopItem* Item) const ;

	//获取Item合成需要的Item
	const FItemCollection* GetIngredientForItem(const UPA_ShopItem* Item) const ;
	
private:

	//存储了当前商店所有Item对应需要的合成Item
	UPROPERTY()
	TMap<const UPA_ShopItem*,FItemCollection> IngredientMap;

	//Item合成表，Key指向了可以合成的Value
	UPROPERTY()
	TMap<const UPA_ShopItem*,FItemCollection> CombinationMap;
	
	//接受LoadAsset委托的回调
	void ShopItemLoadFinished(FStreamableDelegate Callback);

	//为两个Map赋值
	void BuildItemMaps();
	void AddToCombinationMap(const UPA_ShopItem* Ingredient,const UPA_ShopItem* CombinationItem);
};
