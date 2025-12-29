// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayWidget.generated.h"

class UInventoryWidget;
class UAbilitySystemComponent;
class UShopWidget;
class UGameplayAbility;
class UStatsGauge;
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

	void ToggleShop();
private:
	UPROPERTY(meta=(BindWidget))
	class UValueGauge* HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	UValueGauge* ManaBar;

	UPROPERTY(meta=(BindWidget))
	UStatsGauge* AttackDamageGauge;
	
	UPROPERTY(meta=(BindWidget))
	UStatsGauge* ArmorGauge;
	
	UPROPERTY(meta=(BindWidget))
	UStatsGauge* MoveSpeedGauge;

	UPROPERTY(meta=(BindWidget))
	UStatsGauge* IntelligenceGauge;

	UPROPERTY(meta=(BindWidget))
	UStatsGauge* StrengthGauge;
	
	//ListView显示GAIcon
	UPROPERTY(meta=(BindWidget))
	class UAbilityListView* AbilityListView;
	
	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	UPROPERTY(meta=(BindWidget))
	UShopWidget* ShopWidget;

	UPROPERTY(Transient,meta=(BindWidgetAnim))
	UWidgetAnimation* ShopPopupAnimation;

	UPROPERTY(meta=(BindWidget))
	UInventoryWidget* InventoryWidget;
	
	void PlayShopPopupAnimation(bool bPlayForward);
	void SetOwningPawnInputEnabled(bool bPawnInputEnabled);
	void SetShowMouseCursor(bool bShow);
	void SetFocusToGameAndUI();
	void SetFocusToGameOnly();
};
  