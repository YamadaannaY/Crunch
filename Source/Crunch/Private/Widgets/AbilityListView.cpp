// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityListView.h"
#include "Abilities/GameplayAbility.h"
#include "Widgets/AbilityGauge.h"

void UAbilityListView::ConfigureAbilities(const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities)
{
	/**********	AddItem->加入数据源->当需要生成Widget时->OnEntryWidgetGenerated->NativeOnListItemObjectSet		***************/

	//当ListView进行Create或Reuse一个EntryWidget时触发一次
	OnEntryWidgetGenerated().AddUObject(this,&ThisClass::AbilityGaugeGenerated);

	//对所有Abilities进行注册
	for (const TPair<ECAbilityInputID,TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		//Abilities的CDO被加入到ListView的数据源中
		AddItem(AbilityPair.Value.GetDefaultObject());
	}
}

void UAbilityListView::AbilityGaugeGenerated(UUserWidget& Widget)
{
	UAbilityGauge* AbilityGauge=Cast<UAbilityGauge>(&Widget);

	if (AbilityGauge)
	{
		//找到GA对应的WidgetData，从中找到Icon并加载
		AbilityGauge->ConfigureWithWidgetData(FindWidgetDataForAbility(AbilityGauge->GetListItem<UGameplayAbility>()->GetClass()));
	}
}

const FAbilityWidgetData* UAbilityListView::FindWidgetDataForAbility(
	const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
	if (!AbilityDataTable)
	{
		return nullptr;
	}

	//遍历DT，获取行名与其数据指针的Map
	for (auto& AbilityWidgetDataPair :AbilityDataTable->GetRowMap())
	{
		//通过行名找到对应数据指针WidgetData
		const FAbilityWidgetData* WidgetData=AbilityDataTable->FindRow<FAbilityWidgetData>(AbilityWidgetDataPair.Key,"");

		//目标Data
		if (WidgetData->AbilityClass==AbilityClass)
		{
			return WidgetData;
		}
	}
	
	return nullptr;
}
