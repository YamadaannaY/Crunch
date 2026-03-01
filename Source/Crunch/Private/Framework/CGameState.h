#pragma once

#include "CoreMinimal.h"
#include "Character/PA_CharacterDefination.h"
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
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//有点击操作进行，更新PlayerSelection
	void RequestPlayerSelectionChange(const APlayerState* RequestingPlayer , uint8 DesiredSlot);

	//判断当前Id对应的Slot是否已经被占用
	bool IsSlotOccupied(uint8 SlotId) const ;

	//判断当前CharacterDef是否已经被某个Player选择，如果是返回false
	bool IsDefinitionSelected(const UPA_CharacterDefination* Definition) const ;

	//将某个被选择的CharacterDef取消选择，即将对应的Selection对应的Def设置为null
	void SetCharacterDeselected(const UPA_CharacterDefination* DefinitionToDelete);
	
	//返回当前所有添加到Slot的Selection
	const TArray<FPlayerSelection>& GetPlayerSelection() const ;

	//当所有Player都已经选择了Slot创建SelectionArray则true否则false
	bool CanStartHeroSelection() const ;

	//在当前Definition未被其他Player选中的情况下为调用Player赋值
	void SetCharacterSelected(const APlayerState* SelectingPlayer , const UPA_CharacterDefination* SelectedDefinition);
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;
	
	//在服务端更新数据，Rep到客户端并触发UpdatedDelegate
	UFUNCTION()
	void OnRep_PlayerSelectionArray() const ;
};
