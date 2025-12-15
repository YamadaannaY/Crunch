// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbilitiesType.generated.h"

class UGameplayEffect;

//定义InputID,通过ID触发GA
UENUM(BlueprintType)
enum class ECAbilityInputID:uint8
{
	None			UMETA(DisplayName="None"),
	BasicAttacks	UMETA(DisplayName="Basic Attack"),
	AbilityOne		UMETA(DisplayName="Ability One"),
	AbilityTwo		UMETA(DisplayName="Ability Two"),
	AbilityThree	UMETA(DisplayName="Ability Three"),
	AbilityFour		UMETA(DisplayName="Ability Four"),
	AbilityFive		UMETA(DisplayName="Ability Five"),
	AbilitySix		UMETA(DisplayName="Ability Six"),
	Confirm			UMETA(DisplayName="Confirm"),
	Cancel			UMETA(DisplayName="Cancel"),
};

//将GE与PushVel在编辑器中对应配置，调用这个Def来PushTarget可以根据配置的Vel为DamageGE施加作用力
USTRUCT(BlueprintType)
struct FGenericDamageEffectDef
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	FVector PushVelocity;
};
