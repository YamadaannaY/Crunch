#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.generated.h"

class UPA_CharacterDefination;
class UPA_SkinDefination;
class APlayerState;

//结构类，用于存储Player在Lobby选择阶段的数据
USTRUCT()
struct FPlayerSelection
{
	GENERATED_BODY()

	FPlayerSelection();
	FPlayerSelection(uint8 InSlot , const APlayerState* InPlayerState);

	//Set
	FORCEINLINE void SetSlot(uint8 InSlot) {UE_LOG(LogTemp,Warning,TEXT("slot:%d"),InSlot); Slot = InSlot;}
	FORCEINLINE void SetCharacterDefinition(const UPA_CharacterDefination* InCharacterDefinition) {  CharacterDefinition = InCharacterDefinition;}
	FORCEINLINE void SetSkinDefinition(const UPA_SkinDefination* InSkinDefinition) { SkinDefinition = InSkinDefinition; }
	FORCEINLINE void SetHeroConfirmed(bool bConfirmed) { bHeroConfirmed = bConfirmed; }
	//Get
	FORCEINLINE uint8 GetPlayerSlot() const { return Slot;}
	FORCEINLINE FUniqueNetIdRepl GetPlayerUniqueId() const {return PlayerUniqueId;}
	FORCEINLINE FString GetPlayerNickName() const {return PlayerNickName;}
	FORCEINLINE const UPA_CharacterDefination* GetCharacterDefinition() const {return CharacterDefinition;}
	FORCEINLINE const UPA_SkinDefination* GetSkinDefinition() const { return SkinDefinition; }
	FORCEINLINE bool IsHeroConfirmed() const { return bHeroConfirmed; }

	//PlayerSelection中存储了PlayerNickName,将参数State的PlayerName与其对应进行判断
	bool IsForPlayer(const APlayerState* PlayerState) const ;

	//主要判断Slot是否处于合法边界内
	bool IsValid() const ;

	//int8->255
	static uint8 GetInvalidSlot();

private:
		UPROPERTY()
		uint8 Slot;

		UPROPERTY()
		FUniqueNetIdRepl PlayerUniqueId;

		UPROPERTY()
		FString PlayerNickName;

		UPROPERTY()
		const UPA_CharacterDefination* CharacterDefinition;

		/** 当前选择的皮肤（nullptr = 默认皮肤） */
		UPROPERTY()
		const UPA_SkinDefination* SkinDefinition;

		/** 英雄是否已确认（确认后不可再切换英雄） */
		UPROPERTY()
		bool bHeroConfirmed = false;
};
