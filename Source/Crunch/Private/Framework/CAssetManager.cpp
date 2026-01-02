// Fill out your copyright notice in the Description page of Project Settings.


#include "CAssetManager.h"

UCAssetManager& UCAssetManager::Get()
{
	UCAssetManager* Singleton = Cast<UCAssetManager>(GEngine->AssetManager.Get());
	if (Singleton) return *Singleton;

	UE_LOG(LogTemp,Fatal,TEXT("AssetManager needs to be of the type  CAssetManager"));
	
	return (*NewObject<UCAssetManager>());
}

void UCAssetManager::LoadShopItems(const FStreamableDelegate& LoadFinishedCallback)
{
	LoadPrimaryAssetsWithType(UPA_ShopItem::GetShopItemAssetType(),TArray<FName>(),FStreamableDelegate::CreateUObject(this,&ThisClass::ShopItemLoadFinished,LoadFinishedCallback));
}

bool UCAssetManager::GetLoadedShopItem(TArray<const UPA_ShopItem*>& OutItems) const
{
	TArray<UObject*> LoadedObjects;

	//这个函数只会查找所有重写了ID或者在Setting的AssetManager中登记了的DA
	const bool bLoaded=GetPrimaryAssetObjectList(UPA_ShopItem::GetShopItemAssetType(),LoadedObjects);

	if (bLoaded)
	{
		//存储所有ShopItem
		for (UObject* ObjectLoaded : LoadedObjects)
		{
			OutItems.Add(Cast<UPA_ShopItem>(ObjectLoaded));
		}
	}
	return bLoaded;
}

const FItemCollection* UCAssetManager::GetCombinationForItem(const UPA_ShopItem* Item) const
{
	return CombinationMap.Find(Item);
}

const FItemCollection* UCAssetManager::GetIngredientForItem(const UPA_ShopItem* Item) const
{
	return IngredientMap.Find(Item);
}

void UCAssetManager::ShopItemLoadFinished(FStreamableDelegate Callback)
{
	//为调用者提供的异步回调函数
	Callback.ExecuteIfBound();

	//加载完Item对Map赋值
	BuildItemMaps();	
}

void UCAssetManager::BuildItemMaps()
{
	TArray<const UPA_ShopItem*> LoadedItems;

	//获取已经记载的所有Item
	if (GetLoadedShopItem(LoadedItems))
	{
		//对这些Item遍历，再遍历其子Item，为子Item填充CombinationMap,为Item填充IngredientMap
		for (const UPA_ShopItem* Item : LoadedItems)
		{
			if (Item->GetIngredients().Num()==0) continue;

			TArray<const UPA_ShopItem*> Items;

			//遍历其软引用ItemArray并同步加载其指针
			for (const TSoftObjectPtr<UPA_ShopItem>& Ingredient : Item->GetIngredients())
			{
				const UPA_ShopItem* IngredientItem = Ingredient.LoadSynchronous();

				//将子Item加载到Items
				Items.Add(IngredientItem);

				
				AddToCombinationMap(IngredientItem,Item);
			}
			
			IngredientMap.Add(Item,FItemCollection{Items});
		}
	}
}

void UCAssetManager::AddToCombinationMap(const UPA_ShopItem* Ingredient, const UPA_ShopItem* CombinationItem)
{
	FItemCollection* Combinations=CombinationMap.Find(Ingredient);

	if (Combinations)
	{
		if (!Combinations->Contains(CombinationItem))   CombinationMap.Add(CombinationItem);
	}
	else
	{
		CombinationMap.Add(Ingredient,FItemCollection{TArray<const UPA_ShopItem*>{CombinationItem}});
	}
}

