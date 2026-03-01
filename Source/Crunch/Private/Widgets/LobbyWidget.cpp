#include "LobbyWidget.h"

#include "CharacterEntryWidget.h"
#include "Character/PA_CharacterDefination.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TileView.h"
#include "Framework/CGameState.h"
#include "Framework/CAssetManager.h"
#include "Widgets/TeamSelectionWidget.h"
#include "NetWork/NetStatics.h"
#include "Player/CPlayerState.h"
#include "Player/LobbyPlayerController.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearAndPopulateTeamSelectionSlots();

	LobbyPlayerController = GetOwningPlayer<ALobbyPlayerController>();
	if (LobbyPlayerController)
	{
		//某一个客户端点击Button则所有客户端触发此委托
		LobbyPlayerController->OnSwitchToHeroSelection.BindUObject(this,&ThisClass::SwitchToHeroSelection);
	}

	//确保State已经就绪
	ConfigureGameState();

	//true的情况下Button会高亮
	StartHeroSelectionButton->SetIsEnabled(false);
	StartHeroSelectionButton->OnClicked.AddDynamic(this,&ThisClass::StartHeroSelectionButtonClicked);
	
	UCAssetManager::Get().LoadCharacterDefinition(FStreamableDelegate::CreateUObject(this,&ThisClass::CharacterDefinitionLoaded));

	if (CharacterSelectionTileView)
	{
		CharacterSelectionTileView->OnItemSelectionChanged().AddUObject(this,&ThisClass::CharacterSelected);
	}
}

void ULobbyWidget::ClearAndPopulateTeamSelectionSlots()
{
	//Clear
	TeamSelectionSlotGridPanel ->ClearChildren();

	//Populate
	for (int i =0 ; i<UNetStatics::GetPlayerCountPerTeam()*2  ; ++i)
	{
		//为GridSlot配置具体类
		UTeamSelectionWidget* NewSelectionSlot = CreateWidget<UTeamSelectionWidget>(this,TeamSelectionWidgetClass);

		if (NewSelectionSlot)
		{
			NewSelectionSlot ->SetSlotID(i);

			//GridPanel的Slot就是通过Row和Col来确定位置的
			UUniformGridSlot* NewGridSlot = TeamSelectionSlotGridPanel->AddChildToUniformGrid(NewSelectionSlot);
			if (NewGridSlot)
			{
				//Row个数通过阵营决定，Col个数通过阵营人数决定
				int Row= i  % UNetStatics::GetPlayerCountPerTeam();
				int Col = i < UNetStatics::GetPlayerCountPerTeam() ? 0 : 1 ;

				NewGridSlot->SetRow(Row);
				NewGridSlot->SetColumn(Col);
			}

			//连环逻辑的第一步，为Slot的点击进行回调
			NewSelectionSlot->OnSlotClicked.AddUObject(this,&ThisClass::SlotSelected);
			
			TeamSelectionSlots.Add(NewSelectionSlot);
		}
	}
}

void ULobbyWidget::SlotSelected(uint8 NewSlotID)
{
	//服务端将被点击SlotWidget的ID和当前PlayerState进行绑定配置一个PlayerSelection
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_RequestSlotSelectionChange(NewSlotID);
	}
}

void ULobbyWidget::ConfigureGameState()
{
	UWorld* World = GetWorld();
	if (!World) return ;

	CGameState = World->GetGameState<ACGameState>();
	//延迟1秒再次操作GameState
	if (!CGameState)
	{
		World->GetTimerManager().SetTimer(ConfigureGameStateTimerHandle,this,
			&ULobbyWidget::ConfigureGameState,1.f);
	}
	else
	{
		CGameState->OnPlayerSelectionUpdated.AddUObject(this,&ThisClass::UpdatePlayerSelectionOnDisplay);

		//用默认的Selection数组进行一次初始化，默认是没有Selection的，则所有显示Empty
		UpdatePlayerSelectionOnDisplay(CGameState->GetPlayerSelection());
	}
}

void ULobbyWidget::UpdatePlayerSelectionOnDisplay(const TArray<FPlayerSelection>& PlayerSelections)
{
	for (UTeamSelectionWidget* SelectionSlot : TeamSelectionSlots)
	{
		SelectionSlot->UpdateSlotInfo("Empty");
	}

	for (UUserWidget* CharacterEntryAsWidget : CharacterSelectionTileView->GetDisplayedEntryWidgets())
	{
		if (UCharacterEntryWidget* CharacterEntryWidget = Cast<UCharacterEntryWidget>(CharacterEntryAsWidget))
		{
			CharacterEntryWidget->SetSelected(false);
		}
	}

	for (const FPlayerSelection& PlayerSelection : PlayerSelections)
	{
		if (!PlayerSelection.IsValid()) continue;

		TeamSelectionSlots[PlayerSelection.GetPlayerSlot()]->UpdateSlotInfo(PlayerSelection.GetPlayerNickName());

		UCharacterEntryWidget* SelectedEntry = CharacterSelectionTileView->GetEntryWidgetFromItem<UCharacterEntryWidget>(PlayerSelection.GetCharacterDefinition());
		if (SelectedEntry)
		{
			SelectedEntry->SetSelected(true);
		}
	}

	//每次都执行判断函数确定Button是否可以点击
	if (CGameState)
	{
		StartHeroSelectionButton->SetIsEnabled(CGameState->CanStartHeroSelection());
	}
}

void ULobbyWidget::StartHeroSelectionButtonClicked()
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_StartHeroSelection();
	}
}

void ULobbyWidget::SwitchToHeroSelection()
{
	MainSwitcher->SetActiveWidget(HeroSelectionRoot);
}

void ULobbyWidget::CharacterDefinitionLoaded()
{
	TArray<UPA_CharacterDefination*> LoadedCharacterDefinitions;
	
	if (UCAssetManager::Get().GetLoadedCharacterDefinition(LoadedCharacterDefinitions))
	{
			CharacterSelectionTileView->SetListItems(LoadedCharacterDefinitions);
	}
}

void ULobbyWidget::CharacterSelected(UObject* SelectedUObject)
{
	if (!CPlayerState) CPlayerState = GetOwningPlayerState<ACPlayerState>();
	if (!CPlayerState) return ;

	if (const UPA_CharacterDefination* CharacterDefinition = Cast<UPA_CharacterDefination>(SelectedUObject))
	{
		CPlayerState->Server_SetSelectedCharacterDefinition(CharacterDefinition);
	}
}
