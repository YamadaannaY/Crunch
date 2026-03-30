#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayWidget.generated.h"

class UCanvasPanel;
class UWidgetSwitcher;
class USkeletalMeshRenderWidget;
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

	//切换商店界面
	void ToggleShop();
	
	UFUNCTION()
	void ToggleGameplayMenu();
	
	//切换Menu
	void ShowGameplayMenu();
	
	//修改Menu标题文本
	void SetGameplayMenuTitle(const FString& NewTitle);
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

	UPROPERTY(meta=(BindWidget))
	USkeletalMeshRenderWidget* HeadShotWidget;

	UPROPERTY(meta=(BindWidget))
	class UMatchStatWidget* MatchStatWidget;

	UPROPERTY(meta=(BindWidget))
	class UCrosshairWidget* CrosshairWidget;

	UPROPERTY(meta=(BindWidget))
	class UGameplayMenu* GameplayMenu;
	
	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* MainSwitcher;

	UPROPERTY(meta=(BindWidget))
	UCanvasPanel* GameplayWidgetRootPanel;

	UPROPERTY(meta=(BindWidget))
	UCanvasPanel* GameplayMenuRootPanel;

	//播放动画
	void PlayShopPopupAnimation(bool bPlayForward);
	
	//是否允许操控Pawn
	void SetOwningPawnInputEnabled(bool bPawnInputEnabled);
	
	//是否显示鼠标
	void SetShowMouseCursor(bool bShow);
	
	//允许操作UI和游戏，UI优先响应
	void SetFocusToGameAndUI();
	
	//只允许操作游戏
	void SetFocusToGameOnly();
};
  