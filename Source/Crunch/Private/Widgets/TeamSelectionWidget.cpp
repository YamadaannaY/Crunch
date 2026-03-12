#include "TeamSelectionWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UTeamSelectionWidget::SetSlotID(uint8 NewSlotID)
{
	SlotID = NewSlotID;
}

void UTeamSelectionWidget::UpdateSlotInfo(const FString& PlayerNickName)
{
	InfoText -> SetText(FText::FromString(PlayerNickName));
}

void UTeamSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SelectButton->OnClicked.AddDynamic(this,&ThisClass::SelectButtonClicked);
}

void UTeamSelectionWidget::SelectButtonClicked()
{
	UE_LOG(LogTemp,Warning,TEXT("Slot id = %d"),int{SlotID});
	OnSlotClicked.Broadcast(SlotID);
}
