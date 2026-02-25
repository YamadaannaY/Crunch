
#include "LobbyWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Widgets/TeamSelectionWidget.h"
#include "NetWork/NetStatics.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearAndPopulateTeamSelectionSlots();
}

void ULobbyWidget::ClearAndPopulateTeamSelectionSlots()
{
	//Clear
	TeamSelectionSlotGridPanel ->ClearChildren();

	//Populate
	for (int i =0 ; i<UNetStatics::GetPlayerCountPerTeam()*2  ; ++i)
	{
		UTeamSelectionWidget* NewSelectionSlot = CreateWidget<UTeamSelectionWidget>(this,TeamSelectionWidgetClass);

		if (NewSelectionSlot)
		{
			NewSelectionSlot ->SetSlotID(i);
			UUniformGridSlot* NewGridSlot = TeamSelectionSlotGridPanel->AddChildToUniformGrid(NewSelectionSlot);
			if (NewGridSlot)
			{
				int Row= i  % UNetStatics::GetPlayerCountPerTeam();
				int Col = i < UNetStatics::GetPlayerCountPerTeam() ? 0 : 1 ;

				NewGridSlot->SetRow(Row);
				NewGridSlot->SetColumn(Col);
			}

			NewSelectionSlot->OnSlotClicked.AddUObject(this,&ThisClass::SlotSelected);
			TeamSelectionSlots.Add(NewSelectionSlot);
		}
	}
	
}

void ULobbyWidget::SlotSelected(uint8 NewSlotID)
{
	UE_LOG(LogTemp,Warning,TEXT("trying to switch to slot :%d"),NewSlotID);
}


