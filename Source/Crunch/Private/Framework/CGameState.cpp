#include "CGameState.h"
#include "Net/UnrealNetwork.h"

void ACGameState::RequestPlayerSelectionChange(const APlayerState* RequestingPlayer , uint8 DesiredSlot)
{
	//服务端进行Slot的修改操作
	if (!HasAuthority() || IsSlotOccupied(DesiredSlot)) return ;

	FPlayerSelection* PlayerSelectionPtr = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
		{return PlayerSelection.IsForPlayer(RequestingPlayer);});

	//如果当前执行点击操作的Player已经做过点击操作被存储在Array中，则修改其SlotID
	if (PlayerSelectionPtr)
	{
		PlayerSelectionPtr ->SetSlot(DesiredSlot);
	}
	//当前Player没有执行过点击操作，则新建一个Selection，存储当前PlayerState和对应的SlotID
	else
	{
		PlayerSelectionArray.Add(FPlayerSelection(DesiredSlot , RequestingPlayer));
	}
}

bool ACGameState::IsSlotOccupied(uint8 SlotId) const
{
	for (const FPlayerSelection& PlayerSelection : PlayerSelectionArray)
	{
		if (PlayerSelection.GetPlayerSlot() == SlotId)
		{
			return true;
		}
	}
	return false;
}

bool ACGameState::IsDefinitionSelected(const UPA_CharacterDefination* Definition) const
{
	const FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate
	([&](const FPlayerSelection& PlayerSelection)
		{return PlayerSelection.GetCharacterDefinition() == Definition;});

	return FoundPlayerSelection != nullptr;
}

void ACGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ACGameState , PlayerSelectionArray , COND_None , REPNOTIFY_Always)
}

const TArray<FPlayerSelection>& ACGameState::GetPlayerSelection() const
{
	return PlayerSelectionArray;
}

bool ACGameState::CanStartHeroSelection() const
{
	return PlayerSelectionArray.Num() == PlayerArray.Num();
}

void ACGameState::SetCharacterSelected(const APlayerState* SelectingPlayer,
	const UPA_CharacterDefination* SelectedDefinition)
{
	if (IsDefinitionSelected(SelectedDefinition)) return ;

	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
	{
		return PlayerSelection.IsForPlayer(SelectingPlayer);
	});

	if (FoundPlayerSelection)
	{
		FoundPlayerSelection->SetCharacterDefinition(SelectedDefinition);
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
}

void ACGameState::SetCharacterDeselected(const UPA_CharacterDefination* DefinitionToDelete)
{

	if (!DefinitionToDelete) return ;

	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
	{
		return PlayerSelection.GetCharacterDefinition() == DefinitionToDelete;
	});

	if (FoundPlayerSelection)
	{
		FoundPlayerSelection->SetCharacterDefinition(nullptr);
		
		//更新客户端的PlayerSelectionArray
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}

}

void ACGameState::OnRep_PlayerSelectionArray() const
{
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}
