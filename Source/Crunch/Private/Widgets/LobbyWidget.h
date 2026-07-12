//Lobby的显示Widget，附带一个GridPanel，其中包含许多Slot，通过Click交互，对应更新信息

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/PlayerInfoTypes.h"
#include "LobbyWidget.generated.h"

class UAbilityListView;
class UTileView;
struct FPlayerSelection;
class UPA_SkinDefination;
class UPA_CharacterDefination;
class UUniformGridPanel;
class UButton;
class UWidgetSwitcher;

/**
 *
 */

UCLASS()
class ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* MainSwitcher;

	UPROPERTY(meta=(BindWidget))
	UWidget* TeamSelectionRoot;

	UPROPERTY(meta=(BindWidget))
	UWidget* HeroSelectionRoot;

	/** Skin selection panel —— shown after hero is confirmed */
	UPROPERTY(meta=(BindWidget))
	UWidget* SkinSelectionRoot;

	UPROPERTY(meta=(BindWidget))
	UButton* StartHeroSelectionButton;

	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* TeamSelectionSlotGridPanel;

	UPROPERTY(meta=(BindWidget))
	UTileView* CharacterSelectionTileView;

	/** Skin selection TileView —— shows available skins for confirmed hero */
	UPROPERTY(meta=(BindWidget))
	UTileView* SkinSelectionTileView;

	UPROPERTY(meta=(BindWidget))
	UAbilityListView* AbilityListView;

	UPROPERTY(meta=(BindWidget))
	class UPlayerTeamLayoutWidget* PlayerTeamLayoutWidget;

	UPROPERTY(meta=(BindWidget))
	UButton* StartMatchButton;

	/** Confirm hero button —— locks hero and switches to skin selection */
	UPROPERTY(meta=(BindWidget))
	UButton* ConfirmHeroButton;

	UPROPERTY(EditDefaultsOnly,Category="TeamSelection")
	TSubclassOf<class UTeamSelectionWidget> TeamSelectionWidgetClass;

	UPROPERTY()
	TArray<UTeamSelectionWidget*> TeamSelectionSlots;

	UPROPERTY()
	class ALobbyPlayerController* LobbyPlayerController;

	UPROPERTY()
	class ACGameState* CGameState;

	UPROPERTY()
	class ACPlayerState* CPlayerState;

	UPROPERTY(EditDefaultsOnly,Category="Character Display")
	TSubclassOf<class ACharacterDisplay> CharacterDisplayClass;

	UPROPERTY()
	ACharacterDisplay* CharacterDisplay;

	FTimerHandle ConfigureGameStateTimerHandle;

	//清理Panel并重新分配Slot
	void ClearAndPopulateTeamSelectionSlots();

	//当客户端的Slot被点击时，将当前SlotId传给LobbyController以提供给PlayerSelection，让Selection在这个Slot显示
	void SlotSelected(uint8 NewSlotID);

	//配置GameState委托回调，保证触发交互的时候已经有了CGameState
	void ConfigureGameState();

	//绑定一个现有SlotArray被修改的时候响应的函数，更新一次所有Slot的信息
	void UpdatePlayerSelectionOnDisplay(const TArray<FPlayerSelection>& PlayerSelections);

	//当所有人都进行了TeamSelection后才允许调用此函数，为ButtonClick的回调,服务端侧在LobbyController中分发到所有客户端的委托
	UFUNCTION()
	void StartHeroSelectionButtonClicked();

	//分发委托的响应，TeamSelection完毕，切换到HeroSelection
	void SwitchToHeroSelection();

	//加载所有PA_Definition并设置为TileView的ListItem
	void CharacterDefinitionLoaded();

	//CharacterDefListItem被点击时的回调
	void CharacterSelected(UObject* SelectedUObject);

	//初始化创建一个显示Hero动作和外形的DisplayActor类
	void SpawnCharacterDisplay();

	//更新Display类，调用Selection其中的Definition封装好的API进行配置
	void UpdatedCharacterDisplay(const FPlayerSelection& PlayerSelection);

	//MatchButton点击回调
	UFUNCTION()
	void StartMatchButtonClicked();

	// --- Skin Selection ---

	//确认英雄选择按钮回调
	UFUNCTION()
	void ConfirmHeroButtonClicked();

	//当前选中英雄的可用皮肤加载到 TileView 中
	void PopulateSkinSelectionTileView(const UPA_CharacterDefination* CharacterDef);

	//皮肤被选中时的回调
	void SkinSelected(UObject* SelectedUObject);

	//更新皮肤预览（切换 CharacterDisplay 的 Mesh）
	void UpdateSkinPreview(const UPA_SkinDefination* Skin);

	//切换到皮肤选择面板并锁定英雄选择
	void SwitchToSkinSelection();

	/** 当前选中的 CharacterDefinition（缓存用于皮肤加载） */
	UPROPERTY()
	const UPA_CharacterDefination* CurrentSelectedCharacterDef = nullptr;
};
