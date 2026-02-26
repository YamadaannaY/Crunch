#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Player/PlayerInfoTypes.h"
#include "CGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSelectionUpdated , const TArray<FPlayerSelection>& )
/**
 * 
 */
UCLASS()
class CRUNCH_API ACGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	FOnPlayerSelectionUpdated OnPlayerSelectionUpdated;

	//有点击操作进行，更新PlayerSelection
	void RequestPlayerSelectionChange(const APlayerState* RequestingPlayer , uint8 DesiredSlot);

	//判断当前Id对应的Slot是否已经被占用
	bool IsSlotOccupied(uint8 SlotId) const ;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//返回当前所有添加到Slot的Selection
	const TArray<FPlayerSelection>& GetPlayerSelection() const ;
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;

	//在服务端更新数据，Rep到客户端并触发UpdatedDelegate
	UFUNCTION()
	void OnRep_PlayerSelectionArray() const ;
};
