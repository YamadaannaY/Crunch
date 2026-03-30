#include "InventoryContextMenu.h"

FOnButtonClickedEvent& UInventoryContextMenu::GetSellButtonClickedEvent() const
{
	return SellButton->OnClicked;
}

FOnButtonClickedEvent& UInventoryContextMenu::GetUseButtonClickedEvent() const
{
	return UseButton->OnClicked;
}
