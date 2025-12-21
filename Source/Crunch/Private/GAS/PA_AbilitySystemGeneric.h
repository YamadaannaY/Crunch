// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_AbilitySystemGeneric.generated.h"

class UGameplayEffect;
class UGameplayAbility;
/**
 * 
 */
UCLASS()
class UPA_AbilitySystemGeneric : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	FORCEINLINE TSubclassOf<UGameplayEffect> GetFullStateEffect() const {return FullStatEffect;}
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDeathEffect() const {return DeathEffect;}
	
	FORCEINLINE const TArray<TSubclassOf<UGameplayEffect>>& GetInitialEffects() const {return InitialEffects;}
	FORCEINLINE const TArray<TSubclassOf<UGameplayAbility>>& GetPassiveAbilities() const {return PassiveAbilities;}
	FORCEINLINE const UDataTable* GetBaseStatsDataTable() const {return BaseStatsDataTable;}
private:
	//Respawn用，应用满状态GE
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatEffect;

	//死亡用GE，添加Cue，Tag等
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;
	
	//对属性集所有初始属性的InitGE
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	//被动技能
	UPROPERTY(EditDefaultsOnly,Category="Gameplay Abilities")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	//记录Stats属性数值的DT
	UPROPERTY(EditDefaultsOnly,Category="Base Stats")
	UDataTable* BaseStatsDataTable;

};
