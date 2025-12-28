// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "AbilityListView.generated.h"

class UGameplayAbility;
enum class ECAbilityInputID : uint8;
/**
 * 
 */
UCLASS()
class UAbilityListView : public UListView
{
	GENERATED_BODY()
public:
	//对ListView的数据源添加GA，为OnEntryWidgetGenerated绑定回调处理Widget生成后逻辑
	void ConfigureAbilities(const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities);

private:
	//EditDefaultOnly只能在CDO上编辑，也就是类或者蓝图的默认值，UUserWidget本质就是实例，想要在Editor中编辑
	//必须使用EditAnyWhere

	//GA~DT
	UPROPERTY(EditAnywhere,Category="Data")
	UDataTable* AbilityDataTable;

	//OnEntryWidgetGenerated回调函数，当GAWidget被生成时触发
	void AbilityGaugeGenerated(UUserWidget& Widget);

	//遍历存储了Data的DT，遍历每一行Data存储的GAClass，寻找相等的Data并返回
	const struct FAbilityWidgetData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;
};
