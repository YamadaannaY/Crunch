// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "AbilityGauge.generated.h"

struct FOnAttributeChangeData;
class ULevelGauge;
struct FGameplayAbilitySpec;
class UAbilitySystemComponent;

//存储了GAWidget所需数据的结构体,用一个DT统一存储
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

//这是一个Widget类，同时还继承ListEntry（ListView的Item需要用ListEntry才可以在编辑器中可见）
UCLASS()
class UAbilityGauge : public UUserWidget,public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	//构建时调用函数，将Counter隐藏，同时绑定对Cost/Cooldown回调，处理的是整体逻辑
	virtual void NativeConstruct() override;
	
	//当ListView第一次生成EntryWidget时，数据类的这个类被调用，依据数据对Widget进行处理
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

	//ProgressBar变化表示等级
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	FName AbilityLevelParaName="Level";

	//图标变灰表示是否可用
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	FName CanCastAbilityParaName="CanCast";

	//图标闪烁表示是否可以升级
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	FName UpgradePointAvailableParaName="UpgradeAvailable";

	//要覆盖的Image
	UPROPERTY(meta=(BindWidget))
	UImage* Icon;

	UPROPERTY(meta=(BindWidget))
	UImage* LevelGauge;
	
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
	
	float CacheCooldownDuration;
	float CacheCooldownTimeRemaining;

	
	bool bIsAbilityLearned=false;

	//分别对应了完成和冷却两个计时器
	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;

	UPROPERTY()
	const UAbilitySystemComponent* OwnerASCComp;
	
	FGameplayAbilitySpecHandle CacheAbilitySpecHandle;
	
	//Options用于对Text的AsNumber参数进行数位限制
	FNumberFormattingOptions WholeNumberFormattingOptions;
	FNumberFormattingOptions TwoDigitNumberFormattingOptions;

	//当Cost/Cooldown被触发时调用
	void AbilityCommitted(UGameplayAbility* Ability);

	//冷却逻辑
	void StartCooldown(float CooldownTimeRemaining,float CooldownDuration);

	//当Cooldown整体完成后Timer回调
	void CooldownFinished();

	//Cooldown冷却逻辑
	void UpdateCooldown();

	//通过此Gauge中存储的AbilityCDO对应的Class获取Spec
	const FGameplayAbilitySpec* GetAbilitySpec();


	//SpecDirtied广播的回调
	void AbilitySpecUpdated(const FGameplayAbilitySpec& AbilitySpec);

	//等级为0或者Mana值变化时调用，判断是否可以释放技能，改变Material
	void UpdateCanCast();

	//Point属性值的回调，内部调用UpdateCanCast判断，同时改变Material使其闪烁
	void UpgradePointUpdated(const FOnAttributeChangeData& Data);

	//调用UpdateCanCast判断Mana值是否足以释放技能
	void ManaUpdated(const FOnAttributeChangeData& Data);
};

