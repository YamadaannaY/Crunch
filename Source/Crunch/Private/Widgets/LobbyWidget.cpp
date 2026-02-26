#include "LobbyWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Framework/CGameState.h"
#include "Widgets/TeamSelectionWidget.h"
#include "NetWork/NetStatics.h"
#include "Player/LobbyPlayerController.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearAndPopulateTeamSelectionSlots();

	LobbyPlayerController = GetOwningPlayer<ALobbyPlayerController>();

	//确保State已经就绪
	ConfigureGameState();
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

		//用默认的Selection数组进行一次初始化
		UpdatePlayerSelectionOnDisplay(CGameState->GetPlayerSelection());
	}
}

void ULobbyWidget::UpdatePlayerSelectionOnDisplay(const TArray<FPlayerSelection>& PlayerSelections)
{
	for (UTeamSelectionWidget* SelectionSlot : TeamSelectionSlots)
	{
		SelectionSlot->UpdateSlotInfo("Empty");
	}

	for (const FPlayerSelection& PlayerSelection : PlayerSelections)
	{
		if (!PlayerSelection.IsValid()) continue;

		TeamSelectionSlots[PlayerSelection.GetPlayerSlot()]->UpdateSlotInfo(PlayerSelection.GetPlayerNickName());
	}
}
