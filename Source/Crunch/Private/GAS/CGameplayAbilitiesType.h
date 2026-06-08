#pragma once

#include "CoreMinimal.h"
#include "CGameplayAbilitiesType.generated.h"

class UGameplayEffect;

//定义InputID,通过ID触发GA
UENUM(BlueprintType)
enum class ECAbilityInputID:uint8    //可拓展的自定义InputID，用于作为LocalInput函数的参数激活对应的GA
{
	None			UMETA(DisplayName="None"),
	BasicAttacks	UMETA(DisplayName="Basic Attack"),
	Aim				UMETA(DisplayName="Aim"),
	AbilityOne		UMETA(DisplayName="Ability One"),
	AbilityTwo		UMETA(DisplayName="Ability Two"),
	AbilityThree	UMETA(DisplayName="Ability Three"),
	AbilityFour		UMETA(DisplayName="Ability Four"),
	AbilityFive		UMETA(DisplayName="Ability Five"),
	AbilitySix		UMETA(DisplayName="Ability Six"),
	Confirm			UMETA(DisplayName="Confirm"), //ASC初始化中重写为GenericConfirmInputID
	Cancel			UMETA(DisplayName="Cancel"),  //ASC初始化中重写GenericCancelInputID
};

//具有Push效果的DamageGE
USTRUCT(BlueprintType)
struct FGenericDamageEffectDef
{
	GENERATED_BODY()
	
	FGenericDamageEffectDef();
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	FVector PushVelocity;
};

//数值数据配置表
USTRUCT(BlueprintType)
struct FHeroBaseStats :public FTableRowBase
{
	FHeroBaseStats();
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;

	UPROPERTY(EditAnywhere)
	float Strength;

	UPROPERTY(EditAnywhere)
	float Intelligence;

	UPROPERTY(EditAnywhere)
	float StrengthGrowthRate;

	UPROPERTY(EditAnywhere)
	float IntelligenceGrowthRate;

	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;

	UPROPERTY(EditAnywhere)
	float BaseMaxMana;

	UPROPERTY(EditAnywhere)
	float BaseAttackDamage;

	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;

	UPROPERTY(EditAnywhere)
	float BaseGold;
};
