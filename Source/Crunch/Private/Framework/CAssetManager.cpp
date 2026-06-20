#include "CAssetManager.h"
#include "Character/PA_CharacterDefination.h"

UCAssetManager& UCAssetManager::Get()
{
	UCAssetManager* Singleton = Cast<UCAssetManager>(GEngine->AssetManager.Get());
	if (Singleton) return *Singleton;

	UE_LOG(LogTemp,Fatal,TEXT("AssetManager needs to be of the type CAssetManager,now the Get return a NewObject pointer"));
	
	return (*NewObject<UCAssetManager>());
}

void UCAssetManager::LoadShopItems(const FStreamableDelegate& LoadFinishedCallback)
{
	LoadPrimaryAssetsWithType(UPA_ShopItem::GetShopItemAssetType(),TArray<FName>(),FStreamableDelegate::CreateUObject(this,&ThisClass::ShopItemLoadFinished,LoadFinishedCallback));
}

void UCAssetManager::LoadCharacterDefinition(const FStreamableDelegate& LoadFinishedCallback)
{
	LoadPrimaryAssetsWithType(UPA_CharacterDefination::GetCharacterDefinitionAssetType(),TArray<FName>(),LoadFinishedCallback);
}

bool UCAssetManager::GetLoadedShopItem(TArray<const UPA_ShopItem*>& OutItems) const
{
	TArray<UObject*> LoadedObjects;

	//这个函数只会查找所有重写了ID或者在Setting的AssetManager中登记了的PA
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

bool UCAssetManager::GetLoadedCharacterDefinition(TArray<UPA_CharacterDefination*>& LoadedCharacterDefinition) const
{
	TArray<UObject*> LoadedObjects;

	const bool bLoaded = GetPrimaryAssetObjectList(UPA_CharacterDefination::GetCharacterDefinitionAssetType() , LoadedObjects);
	if (bLoaded)
	{
		for (UObject* ObjectLoaded : LoadedObjects)
		{
			LoadedCharacterDefinition.Add(Cast<UPA_CharacterDefination>(ObjectLoaded));
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
	//执行调用者提供的异步回调函数
	Callback.ExecuteIfBound();

	//加载完Item对Map赋值
	BuildItemMaps();	
}

void UCAssetManager::BuildItemMaps()
{
	TArray<const UPA_ShopItem*> LoadedItems;

	//获取所有ShopItems
	if (GetLoadedShopItem(LoadedItems))
	{
		//对这些Item遍历，再遍历其子Item，为子Item填充CombinationMap,为Item填充IngredientMap
		for (const UPA_ShopItem* Item : LoadedItems)
		{
			if (Item->GetIngredients().Num()==0) continue;

			TArray<const UPA_ShopItem*> Items;

			//遍历其软引用ItemArray并同步加载
			for (const TSoftObjectPtr<UPA_ShopItem>& Ingredient : Item->GetIngredients())
			{
				const UPA_ShopItem* IngredientItem = Ingredient.LoadSynchronous();
				if (IngredientItem)
				{
					//创建一个新的IngredientMap值
					Items.Add(IngredientItem);
				
					//更新CombinationMap
					AddToCombinationMap(IngredientItem,Item);
				}
				else
				{
					UE_LOG(LogTemp,Warning,TEXT("Failed to load ingredient for item :%s"),*Item->GetName());
				}
			}
			
			//更新IngredientMap
			IngredientMap.Add(Item,FItemCollection{Items});
		}
	}
}

void UCAssetManager::AddToCombinationMap(const UPA_ShopItem* Ingredient, const UPA_ShopItem* CombinationItem)
{
	
	//如果在合成表中找到Ingredient对应的集合，就从中添加合成Item，否则是第一次对此Item创建CombinationMap创建此集合
	if (FItemCollection* Combinations=CombinationMap.Find(Ingredient))
	{
		if (!Combinations->Contains(CombinationItem))
		{
			Combinations->AddItem(CombinationItem);
		}
	}
	//第一次记录此Combination，需要手动Add一次此映射关系
	else
	{
		CombinationMap.Add(Ingredient,FItemCollection{TArray<const UPA_ShopItem*>{CombinationItem}});
	}
}