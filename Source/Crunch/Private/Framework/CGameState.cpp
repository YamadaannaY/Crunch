#include "CGameState.h"
#include "Net/UnrealNetwork.h"

void ACGameState::RequestPlayerSelectionChange(const APlayerState* RequestingPlayer , uint8 DesiredSlot)
{
	//服务端进行Slot的修改操作
	if (!HasAuthority() || IsSlotOccupied(DesiredSlot)) return ;

	//将规则应用到Array中，有则返回，否则nullptr
	FPlayerSelection* PlayerSelectionPtr = PlayerSelectionArray.FindByPredicate
	([&](const FPlayerSelection& PlayerSelection){return PlayerSelection.IsForPlayer(RequestingPlayer);});
	
	//如果存在，则修改其Slot
	if (PlayerSelectionPtr)
	{
		PlayerSelectionPtr ->SetSlot(DesiredSlot);
	}
	//此Player并未创建过PlayerSelection,此时新建一个Selection
	else
	{
		PlayerSelectionArray.Add(FPlayerSelection(DesiredSlot , RequestingPlayer));
	}
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
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
	([&](const FPlayerSelection& PlayerSelection){return PlayerSelection.GetCharacterDefinition() == Definition;});

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

	//找到当前玩家对应的Selection并传入
	FPlayerSelection* FoundPlayerSelection = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection)
	{
		return PlayerSelection.IsForPlayer(SelectingPlayer);
	});

	if (FoundPlayerSelection)
	{
		//Widget中响应此回调，根据Def更新HeroSlot
		FoundPlayerSelection->SetCharacterDefinition(SelectedDefinition);
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}
}

bool ACGameState::CanStartMatch() const
{
	for (const FPlayerSelection& PlayerSelection : PlayerSelectionArray)
	{
		if (PlayerSelection.GetCharacterDefinition() == nullptr) return false ; 
	}
	
	return true;
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
		
		//Widget中响应此回调，更新HeroSlot
		OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
	}

}

void ACGameState::OnRep_PlayerSelectionArray() const
{
	////Widget中响应此回调
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}