#include "PlayerTeamLayoutWidget.h"
#include "Widgets/PlayerTeamSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "NetWork/NetStatics.h"
#include "Player/PlayerInfoTypes.h"

void UPlayerTeamLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	TeamOneLayoutBox->ClearChildren();
	TeamTwoLayoutBox->ClearChildren();
	
	if (!PlayerTeamSlotWidgetClass) return ;
	
	for (int i=0; i <UNetStatics::GetPlayerCountPerTeam()*2 ; ++i)
	{
		UPlayerTeamSlotWidget* NewSlotWidget = CreateWidget<UPlayerTeamSlotWidget>(GetOwningPlayer(),PlayerTeamSlotWidgetClass);
		TeamSlotWidgets.Add(NewSlotWidget);
		
		UHorizontalBoxSlot* NewSlot;
		if (i<UNetStatics::GetPlayerCountPerTeam())
		{
			NewSlot = TeamOneLayoutBox->AddChildToHorizontalBox(NewSlotWidget);
		}
		else
		{
			NewSlot=TeamTwoLayoutBox->AddChildToHorizontalBox(NewSlotWidget);
		}
		
		NewSlot->SetPadding(FMargin(PlayerTeamWidgetSlotMargin));
	}
}

void UPlayerTeamLayoutWidget::UpdatePlayerSelection(const TArray<FPlayerSelection>& PlayerSelections)
{
	for (UPlayerTeamSlotWidget* SlotWidget : TeamSlotWidgets)
	{
		SlotWidget->UpdateSlot("",nullptr);
	}
	for (const FPlayerSelection& PlayerSelection : PlayerSelections)
	{
		if (!PlayerSelection.IsValid()) continue;
			
		TeamSlotWidgets[PlayerSelection.GetPlayerSlot()]->UpdateSlot(PlayerSelection.GetPlayerNickName(),PlayerSelection.GetCharacterDefinition());
	}
}
