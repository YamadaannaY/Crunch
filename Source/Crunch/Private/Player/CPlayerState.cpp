#include "CPlayerState.h"
#include  "Framework/CGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ACPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACPlayerState , PlayerSelection);
}

void ACPlayerState::BeginPlay()
{
	Super::BeginPlay();

	CGameState = Cast<ACGameState>(UGameplayStatics::GetGameState(this));
	//变化来自服务端，意味着服务端的数据已经处理好了
	if (CGameState && !HasAuthority())
	{
		CGameState->OnPlayerSelectionUpdated.AddUObject(this,&ThisClass::PlayerSelectionUpdated);
	}
}

ACPlayerState::ACPlayerState()
{
	bReplicates = true;
	SetNetUpdateFrequency(100.f);
}

void ACPlayerState::Server_SetSelectedCharacterDefinition_Implementation(const UPA_CharacterDefination* NewDefinition)
{
	if (!CGameState) return ;
	if (!NewDefinition) return ;
	if (CGameState->IsDefinitionSelected(NewDefinition)) return ;
	
	if (PlayerSelection.GetCharacterDefinition())
	{
		CGameState->SetCharacterDeselected(PlayerSelection.GetCharacterDefinition());
	}
	//服务端赋值
	PlayerSelection.SetCharacterDefinition(NewDefinition);
	//客户端同步赋值
	CGameState->SetCharacterSelected(this,NewDefinition);
}

bool ACPlayerState::Server_SetSelectedCharacterDefinition_Validate(const UPA_CharacterDefination* NewDefinition)
{
	return true;
}

void ACPlayerState::PlayerSelectionUpdated(const TArray<FPlayerSelection>& NewPlayerSelections)
{
	for (const FPlayerSelection& NewPlayerSelection : NewPlayerSelections)
	{
		if (NewPlayerSelection.IsForPlayer(this))
		{
			PlayerSelection = NewPlayerSelection;
			break;
		}
	}
}
