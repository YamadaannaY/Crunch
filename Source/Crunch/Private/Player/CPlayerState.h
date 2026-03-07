
#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "PlayerInfoTypes.h"
#include "GameFramework/PlayerState.h"
#include "CPlayerState.generated.h"

class UPA_CharacterDefination;

/**
 * 
 */

UCLASS()
class CRUNCH_API ACPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void BeginPlay() override;
	
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
	ACPlayerState();

	//配置Selection中的Definition，同步处理了客户端的Def
	UFUNCTION(Server, Reliable,WithValidation)
	void Server_SetSelectedCharacterDefinition(const UPA_CharacterDefination* NewDefinition);
	
	TSubclassOf<APawn> GetSelectedPawnClass() const;
	
	FGenericTeamId GetTeamIdBaseOnSlot() const;
private:
	UPROPERTY(Replicated)
	FPlayerSelection PlayerSelection;

	UPROPERTY()
	class ACGameState* CGameState;
	
	//将NewSelections中此Player存储的PlayerSelection更新
	void PlayerSelectionUpdated(const TArray<FPlayerSelection>& NewPlayerSelections);
};
