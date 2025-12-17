// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayWidget.generated.h"

class UGameplayAbility;
enum class ECAbilityInputID : uint8;
struct FGenericTeamId;
/**
 * 
 */
UCLASS()
class UGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//为本地UIBar调用SetAndBoundToGameplayAttribute，更新Percent和Text
	virtual void NativeConstruct() override;

	//将所有GA作为List数据源传入
	void ConfigureAbilities(const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities);
private:
	UPROPERTY(meta=(BindWidget))
	class UValueGauge* HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	UValueGauge* ManaBar;

	//一个List，显示GAIcon
	UPROPERTY(meta=(BindWidget))
	class UAbilityListView* AbilityListView;

	UPROPERTY()
	class UAbilitySystemComponent* OwnerAbilitySystemComponent;
	
};
