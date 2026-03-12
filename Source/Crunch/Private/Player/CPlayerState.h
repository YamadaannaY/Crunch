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
	ACPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void BeginPlay() override;
	
	//服务端调用的Copy函数，用于SeamlessTravel中继承当前Level中PlayerState的特定对象，其余会被重置
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
	//配置Selection中的Definition，同步处理了客户端的Def
	UFUNCTION(Server, Reliable,WithValidation)
	void Server_SetSelectedCharacterDefinition(const UPA_CharacterDefination* NewDefinition);
	
	//获取当前CharacterClass
	TSubclassOf<APawn> GetSelectedPawnClass() const;
	
	//根据当前PlayerSlot的大小获取TeamId
	FGenericTeamId GetTeamIdBaseOnSlot() const;
private:
	UPROPERTY(Replicated)
	FPlayerSelection PlayerSelection;

	UPROPERTY()
	class ACGameState* CGameState;
	
	//将NewSelections中此Player存储的PlayerSelection更新
	void PlayerSelectionUpdated(const TArray<FPlayerSelection>& NewPlayerSelections);
};
