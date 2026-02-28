//Lobby的显示Widget，附带一个GridPanel，其中包含许多Slot，通过Click交互，对应更新信息

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

struct FPlayerSelection;
class UUniformGridPanel;
class UButton;
class UWidgetSwitcher;

/**
 * 
 */

UCLASS()
class CRUNCH_API ULobbyWidget : public UUserWidget
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

	UPROPERTY(meta=(BindWidget))
	UButton* StartHeroSelectionButton;

	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* TeamSelectionSlotGridPanel;

	UPROPERTY(EditDefaultsOnly,Category="TeamSelection")
	TSubclassOf<class UTeamSelectionWidget> TeamSelectionWidgetClass;

	UPROPERTY()
	TArray<UTeamSelectionWidget*> TeamSelectionSlots;

	UPROPERTY()
	class ALobbyPlayerController* LobbyPlayerController;

	UPROPERTY()
	class ACGameState* CGameState;

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

	void CharacterDefinitionLoaded();
};
