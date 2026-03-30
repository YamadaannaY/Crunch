//Type类，实现了用于Player存储Info的结构体数据

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.generated.h"

class UPA_CharacterDefination;
class APlayerState;

USTRUCT()
struct FPlayerSelection
{
	GENERATED_BODY()
	
	FPlayerSelection();
	FPlayerSelection(uint8 InSlot , const APlayerState* InPlayerState);

	//Set
	FORCEINLINE void SetSlot(uint8 InSlot) {UE_LOG(LogTemp,Warning,TEXT("slot:%d"),InSlot); Slot = InSlot;}
	FORCEINLINE void SetCharacterDefinition(const UPA_CharacterDefination* InCharacterDefinition) {  CharacterDefinition = InCharacterDefinition;}
	//Get
	FORCEINLINE uint8 GetPlayerSlot() const { return Slot;}
	FORCEINLINE FUniqueNetIdRepl GetPlayerUniqueId() const {return PlayerUniqueId;}
	FORCEINLINE FString GetPlayerNickName() const {return PlayerNickName;}
	FORCEINLINE const UPA_CharacterDefination* GetCharacterDefinition() const {return CharacterDefinition;}
	
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
};
