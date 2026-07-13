#include "LobbyWidget.h"
#include "AbilityListView.h"
#include "Widgets/CharacterDisplay.h"
#include "CharacterEntryWidget.h"
#include "SkinEntryWidget.h"
#include "PlayerTeamLayoutWidget.h"
#include "Character/PA_CharacterDefination.h"
#include "Character/PA_SkinDefination.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TileView.h"
#include "Framework/CGameState.h"
#include "Framework/CAssetManager.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/TeamSelectionWidget.h"
#include "NetWork/NetStatics.h"
#include "Player/CPlayerState.h"
#include "Player/LobbyPlayerController.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearAndPopulateTeamSelectionSlots();

	//确保State已经就绪
	ConfigureGameState();

	LobbyPlayerController = GetOwningPlayer<ALobbyPlayerController>();
	if (LobbyPlayerController)
	{
		//某一个客户端点击Button则所有客户端触发此委托
		LobbyPlayerController->OnSwitchToHeroSelection.BindUObject(this,&ThisClass::SwitchToHeroSelection);
	}

	//true的情况下Button会高亮
	StartHeroSelectionButton->SetIsEnabled(false);
	StartHeroSelectionButton->OnClicked.AddDynamic(this,&ThisClass::StartHeroSelectionButtonClicked);

	StartMatchButton->SetIsEnabled(false);
	StartMatchButton->OnClicked.AddDynamic(this,&ThisClass::StartMatchButtonClicked);

	// Confirm hero button
	if (ConfirmHeroButton)
	{
		ConfirmHeroButton->SetIsEnabled(false);
		ConfirmHeroButton->OnClicked.AddDynamic(this, &ThisClass::ConfirmHeroButtonClicked);
	}

	UCAssetManager::Get().LoadCharacterDefinition(FStreamableDelegate::CreateUObject(this,&ThisClass::CharacterDefinitionLoaded));

	if (CharacterSelectionTileView)
	{
		CharacterSelectionTileView->OnItemSelectionChanged().AddUObject(this,&ThisClass::CharacterSelected);
	}

	// Bind skin selection callback
	if (SkinSelectionTileView)
	{
		SkinSelectionTileView->OnItemSelectionChanged().AddUObject(this, &ThisClass::SkinSelected);
	}

	// Hide skin selection panel initially
	if (SkinSelectionRoot)
	{
		SkinSelectionRoot->SetVisibility(ESlateVisibility::Collapsed);
	}

	SpawnCharacterDisplay();
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

	// Reset skin selection highlights
	if (SkinSelectionTileView)
	{
		for (UUserWidget* SkinEntryAsWidget : SkinSelectionTileView->GetDisplayedEntryWidgets())
		{
			if (USkinEntryWidget* SkinEntryWidget = Cast<USkinEntryWidget>(SkinEntryAsWidget))
			{
				SkinEntryWidget->SetSelected(false);
			}
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

		// Highlight selected skin
		if (PlayerSelection.GetSkinDefinition())
		{
			USkinEntryWidget* SelectedSkinEntry = SkinSelectionTileView->GetEntryWidgetFromItem<USkinEntryWidget>(PlayerSelection.GetSkinDefinition());
			if (SelectedSkinEntry)
			{
				SelectedSkinEntry->SetSelected(true);
			}
		}

		if (PlayerSelection.IsForPlayer(GetOwningPlayerState()))
		{
			UpdatedCharacterDisplay(PlayerSelection);

			// If hero is already confirmed, switch to skin selection for this player
			if (PlayerSelection.IsHeroConfirmed())
			{
				// Disable hero TileView
				CharacterSelectionTileView->SetVisibility(ESlateVisibility::Collapsed);

				// Show skin selection
				SwitchToSkinSelection();

				CurrentSelectedCharacterDef = PlayerSelection.GetCharacterDefinition();

				// Populate skins (only once, when empty)
				if (PlayerSelection.GetCharacterDefinition()
					&& SkinSelectionTileView
					&& SkinSelectionTileView->GetNumItems() == 0)
				{
					PopulateSkinSelectionTileView(PlayerSelection.GetCharacterDefinition());
				}
			}
		}
	}

	//每次都执行判断函数确定Button是否可以点击
	if (CGameState)
	{
		StartHeroSelectionButton->SetIsEnabled(CGameState->CanStartHeroSelection());
		StartMatchButton->SetIsEnabled(CGameState->CanStartMatch());
	}

	if (PlayerTeamLayoutWidget)
	{
		PlayerTeamLayoutWidget->UpdatePlayerSelection(PlayerSelections);
	}

	// Update confirm button state: enabled when current player has a hero selected but not yet confirmed
	if (ConfirmHeroButton && !CPlayerState) CPlayerState = GetOwningPlayerState<ACPlayerState>();
	if (ConfirmHeroButton && CPlayerState)
	{
		const FPlayerSelection* MySelection = PlayerSelections.FindByPredicate(
			[&](const FPlayerSelection& PS) { return PS.IsForPlayer(CPlayerState); });
		if (MySelection)
		{
			const bool bHasHero = MySelection->GetCharacterDefinition() != nullptr;
			const bool bConfirmed = MySelection->IsHeroConfirmed();
			ConfirmHeroButton->SetIsEnabled(bHasHero && !bConfirmed);
		}
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

	// Ensure skin panel is hidden when entering hero selection
	if (SkinSelectionRoot)
	{
		SkinSelectionRoot->SetVisibility(ESlateVisibility::Collapsed);
	}
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
		CurrentSelectedCharacterDef = CharacterDefinition;
	}
}

void ULobbyWidget::SpawnCharacterDisplay()
{
	if (CharacterDisplay) return ;
	if (!CharacterDisplayClass) return ;

	FTransform CharacterDisplayTransform = FTransform::Identity;

	AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(),APlayerStart::StaticClass());
	if (PlayerStart)
	{
		CharacterDisplayTransform = PlayerStart->GetActorTransform();
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CharacterDisplay = GetWorld()->SpawnActor<ACharacterDisplay>(CharacterDisplayClass,SpawnParams);
	CharacterDisplay->SetActorTransform(CharacterDisplayTransform);
	GetOwningPlayer()->SetViewTarget(CharacterDisplay);
}

void ULobbyWidget::UpdatedCharacterDisplay(const FPlayerSelection& PlayerSelection)
{
	if (!PlayerSelection.GetCharacterDefinition()) return;

	CharacterDisplay->ConfigureWithCharacterDefinition(PlayerSelection.GetCharacterDefinition());

	// Apply skin mesh if selected
	const UPA_SkinDefination* Skin = PlayerSelection.GetSkinDefinition();
	if (Skin)
	{
		USkeletalMesh* SkinMesh = PlayerSelection.GetCharacterDefinition()->LoadDisplayMeshForSkin(Skin);
		if (SkinMesh)
		{
			CharacterDisplay->SetSkinMesh(SkinMesh);
		}
	}

	AbilityListView->ClearListItems();
	const TMap<ECAbilityInputID , TSubclassOf<UGameplayAbility>>* Abilities = PlayerSelection.GetCharacterDefinition()->GetAbilities();
	if (Abilities)
	{
		AbilityListView->ConfigureAbilities(*Abilities);
	}
}

void ULobbyWidget::StartMatchButtonClicked()
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_RequestionStartMatch();
	}
}

// --- Skin Selection ---

void ULobbyWidget::ConfirmHeroButtonClicked()
{
	if (!CPlayerState) CPlayerState = GetOwningPlayerState<ACPlayerState>();
	if (!CPlayerState) return;

	// Send confirm RPC to server
	CPlayerState->Server_ConfirmHeroSelection();
}

void ULobbyWidget::SwitchToSkinSelection()
{
	// Remove hero TileView from layout, skin panel takes its place
	CharacterSelectionTileView->SetVisibility(ESlateVisibility::Collapsed);

	if (SkinSelectionRoot)
	{
		SkinSelectionRoot->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULobbyWidget::PopulateSkinSelectionTileView(const UPA_CharacterDefination* CharacterDef)
{
	if (!SkinSelectionTileView || !CharacterDef) return;

	SkinSelectionTileView->ClearListItems();

	const TArray<TSoftObjectPtr<UPA_SkinDefination>>& AvailableSkins = CharacterDef->GetAvailableSkins();
	if (AvailableSkins.IsEmpty()) return;

	TArray<UPA_SkinDefination*> LoadedSkins;
	for (const TSoftObjectPtr<UPA_SkinDefination>& SkinPtr : AvailableSkins)
	{
		UPA_SkinDefination* LoadedSkin = SkinPtr.LoadSynchronous();
		if (LoadedSkin)
		{
			LoadedSkins.Add(LoadedSkin);
		}
	}

	SkinSelectionTileView->SetListItems(LoadedSkins);
}

void ULobbyWidget::SkinSelected(UObject* SelectedUObject)
{
	if (!CPlayerState) CPlayerState = GetOwningPlayerState<ACPlayerState>();
	if (!CPlayerState) return;

	const UPA_SkinDefination* SkinDef = Cast<UPA_SkinDefination>(SelectedUObject);
	if (!SkinDef) return;

	// Send skin selection to server
	CPlayerState->Server_SetSelectedSkin(SkinDef);

	// Update local preview immediately
	UpdateSkinPreview(SkinDef);

	//立即修改本客户端的 CDO Mesh
	const UPA_CharacterDefination* CharDef = CurrentSelectedCharacterDef;
	if (!CharDef) CharDef = CPlayerState->GetPlayerSelection().GetCharacterDefinition();
	if (CharDef)
	{
		CharDef->ApplySkinToClassDefault(SkinDef);
	}
}

void ULobbyWidget::UpdateSkinPreview(const UPA_SkinDefination* Skin)
{
	if (!CharacterDisplay) return;

	// Use current selected character definition to get the skin mesh
	const UPA_CharacterDefination* CharDef = CurrentSelectedCharacterDef;
	if (!CharDef && CPlayerState)
	{
		CharDef = CPlayerState->GetPlayerSelection().GetCharacterDefinition();
	}

	if (CharDef)
	{
		USkeletalMesh* SkinMesh = CharDef->LoadDisplayMeshForSkin(Skin);
		if (SkinMesh)
		{
			CharacterDisplay->SetSkinMesh(SkinMesh);
		}
	}
}
