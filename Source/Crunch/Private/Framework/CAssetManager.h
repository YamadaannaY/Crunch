// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/PA_ShopItem.h"
#include "Engine/AssetManager.h"
#include "CAssetManager.generated.h"

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

	//一键加载属于特定Type的所有资源，这里用于加载ShopItem,并且绑定一个回调函数，这个函数额外带有一个Delegate,用于加载此函数的对象能够实现自己想要调用的异步回调函数
	void LoadShopItems(const FStreamableDelegate& LoadFinishedCallback);

	//获得当前商店内所有被加载的Item
	bool GetLoadedShopItem(TArray<const UPA_ShopItem*>& OutItems) const ;


private:
	//接受LoadAsset委托的回调
	void ShopItemLoadFinished(FStreamableDelegate Callback);

	//为两个Map赋值
	void BuildItemMaps();
	void AddToCombinationMap(const UPA_ShopItem* Ingredient,const UPA_ShopItem* CombinationItem);

	//存储了当前商店所有Item对应需要的合成Item
	UPROPERTY()
	TMap<const UPA_ShopItem*,FItemCollection> IngredientMap;

	//Item合成表，Key指向了可以合成的Value
	UPROPERTY()
	TMap<const UPA_ShopItem*,FItemCollection> CombinationMap;

};
