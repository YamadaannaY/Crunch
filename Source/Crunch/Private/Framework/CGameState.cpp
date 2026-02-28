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

void ACGameState::OnRep_PlayerSelectionArray() const
{
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}
