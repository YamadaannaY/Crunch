// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/ShopItemWidget.h"
#include "Inventory/PA_ShopItem.h"
#include "Components/ListView.h"
#include "Framework/CAssetManager.h"

UUserWidget* UShopItemWidget::GetWidget() const
{
	//复制一个当前的ItemWidget类
	UShopItemWidget* Copy=CreateWidget<UShopItemWidget>(GetOwningPlayer(),GetClass());
	Copy->CopyFromOther(this);
	return Copy;
}

TArray<const ITreeNodeInterface*> UShopItemWidget::GetInputs() const
{
	//获得当前Item可合成Item的集合
	const FItemCollection* Collection=UCAssetManager::Get().GetCombinationForItem(GetShopItem());
	if (Collection)
	{
		//返回所有可合成Item的TreeNode接口
		return ItemsToInterfaces(Collection->GetItems());
	}
	return TArray<const ITreeNodeInterface*>{};
}

TArray<const ITreeNodeInterface*> UShopItemWidget::GetOutputs() const
{
	//获取当前Item合成需要的子Item的集合
	const FItemCollection* Collection=UCAssetManager::Get().GetIngredientForItem(GetShopItem());
	if (Collection)
	{
		//返回这些子Items的TreeNode接口
		return ItemsToInterfaces(Collection->GetItems());
	}
	return TArray<const ITreeNodeInterface*>{};
}

const UObject* UShopItemWidget::GetItemObject() const
{
	return ShopItem;
}

void UShopItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	InitWithShopItem(Cast<UPA_ShopItem>(ListItemObject));
	ParentListView=Cast<UListView>(GetOwningListView());
}

void UShopItemWidget::CopyFromOther(const UShopItemWidget* OtherWidget)
{
	//在Tree上显示的ItemWidget应该具有的功能：能够购买，能够左键唤出TreeWidget,具有包含这个Widget的ListView，并且本身显示Icon和ToolTip
	OnItemPurchaseIssued=OtherWidget->OnItemPurchaseIssued;
	OnShopItemClicked=OtherWidget->OnShopItemClicked;
	ParentListView=OtherWidget->ParentListView;
	InitWithShopItem(OtherWidget->GetShopItem());
}

void UShopItemWidget::InitWithShopItem(const UPA_ShopItem* NewShopItem)
{
	
	//获取ListObject
	ShopItem=NewShopItem;
	
	if (!ShopItem) return ;
	
	SetIcon(ShopItem->GetIcon());
	SetToolTipWidget(ShopItem);
}

TArray<const ITreeNodeInterface*> UShopItemWidget::ItemsToInterfaces(const TArray<const UPA_ShopItem*>& Items) const
{
	TArray<const ITreeNodeInterface*> RetInterfaces;
	if (!ParentListView) return RetInterfaces;

	for (const UPA_ShopItem* Item : Items)
	{
		//从存储了所有ItemWidget的ListView中获取PA对应的Widget，将Widget接口添加到RetInterfaces
		const UShopItemWidget* ItemWidget=ParentListView->GetEntryWidgetFromItem<UShopItemWidget>(Item);
		if (ItemWidget)
		{
			RetInterfaces.Add(ItemWidget);
		}
	}
	return RetInterfaces;
}

void UShopItemWidget::RightButtonClicked()
{
	OnItemPurchaseIssued.Broadcast(GetShopItem());
}

void UShopItemWidget::LeftButtonClicked()
{
	OnShopItemClicked.Broadcast(this);
}
