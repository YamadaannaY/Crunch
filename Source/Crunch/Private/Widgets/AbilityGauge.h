// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "AbilityGauge.generated.h"

//存储了GAWidget的数据结构体,用一个DT存储
USTRUCT(BlueprintType)
struct FAbilityWidgetData :public FTableRowBase
{
	GENERATED_BODY()

	//GAClass
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> AbilityClass;

	//GAName
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FName AbilityName;

	//软引用，只在需要时加载
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
 
	//对GA的文本描述
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FText Description;
};

/**
 * 
 */

class UImage;
class UTextBlock;

//这是一个Widget类，同时还继承ListEntry（ListView的Item需要用ListEntry才可以在Editor中自定义编辑）
UCLASS()
class UAbilityGauge : public UUserWidget,public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	//Gauge构建时调用函数，将Counter隐藏，同时绑定对Cost/Cooldown回调，处理的是整体逻辑
	virtual void NativeConstruct() override;
	
	//在UserWidget被ListView绑定为某个Item或复用后重新绑定新Item时调用，这里处理了对UI的细节逻辑
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	//将Data中存储的Icon加载并赋予IconMaterialParaName
	void ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData);
	
private:
	UPROPERTY(EditDefaultsOnly,Category="Cooldown")
	float CooldownUpdateInterval=0.1f;
	
	//图标的Material中的参数名
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	FName IconMaterialParaName="Icon";

	//Material中的渲染参数，用于显示阴影百分比变化
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	FName CooldownPercentParaName="Percent";

	//要覆盖的Image
	UPROPERTY(meta=(BindWidget))
	UImage* Icon;

	//技能冷却剩余时间的计数文本
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CooldownCounterText;

	//冷却总时间的计数文本
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CooldownDurationText;

	//消耗Mana的数值文本
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CostText;

	//这个Gauge对应的GAClass
	UPROPERTY()
	UGameplayAbility* AbilityCDO;

	//当Cost/Cooldown被触发时调用
	void AbilityCommitted(UGameplayAbility* Ability);

	//冷却逻辑
	void StartCooldown(float CooldownTimeRemaining,float CooldownDuration);
	
	float CacheCooldownDuration;
	float CacheCooldownTimeRemaining;

	//分别对应了完成和冷却两个计时器
	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;

	//当Cooldown整体完成后Timer回调
	void CooldownFinished();

	//Cooldown冷却逻辑
	void UpdateCooldown();

	//Options用于对Text的AsNumber参数进行数位限制
	FNumberFormattingOptions WholeNumberFormattingOptions;
	FNumberFormattingOptions TwoDigitNumberFormattingOptions;
};

