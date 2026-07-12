#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Player/PlayerInfoTypes.h"
#include "CGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSelectionUpdated , const TArray<FPlayerSelection>& )

class UPA_CharacterDefination;
class UPA_SkinDefination;

/*
保存并同步整个游戏对所有玩家可见的状态数据，适合用作保存所有玩家共享并需要的数据，例如得分，阵营，游戏状态，时间等等，
还有一个关键的PlayerArray存储了所有的APlayerState，存在于客户端和服务端，自动复制，客户端也能调用以制作UI等，
在这里，GameState主要存储了一个PlayerSelectionArray以及配置对应的交互逻辑，主要是因为客户端需要所有其他客户端的PlayerSelectionArray用来更新SlotUI
除此之外，还有PlayerSelection数组的相关操作函数
*/
UCLASS()
class CRUNCH_API ACGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	//用于更新PlayerSelectionArray时的响应回调委托，包括Slot、Def的变化，在UI中更新
	FOnPlayerSelectionUpdated OnPlayerSelectionUpdated;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//有点击操作，更新PlayerSelection
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

	//有Def在PlayerState中被更新，将Array也中对应的PlayerSelection也更新一次
	void SetCharacterSelected(const APlayerState* SelectingPlayer , const UPA_CharacterDefination* SelectedDefinition);
	
	//是否所有玩家选择Hero，可以开始比赛
	bool CanStartMatch() const ;

	//设置玩家的皮肤选择
	void SetSkinSelected(const APlayerState* SelectingPlayer, const UPA_SkinDefination* NewSkin);

	//确认英雄选择（锁定英雄，不可再切换）
	void SetHeroConfirmed(const APlayerState* SelectingPlayer);

private:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;
	
	//在服务端更新数据，Rep到GameState客户端并触发UpdatedDelegate
	UFUNCTION()
	void OnRep_PlayerSelectionArray() const ;
};
