// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "CGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UCGameplayAbility();
protected:
	//通过MeshComp获得AnimInstance
	UAnimInstance* GetOwnerAnimInstance() const;

	//以AN中记录的起始点和结束点为参数进行SphereTrace
	TArray<FHitResult> GetHitResultsFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle,float SphereSweepRadius=30.f,ETeamAttitude::Type TargetTeam=ETeamAttitude::Hostile,bool bShowDebug=false,bool bIgnoreSelf=true);

	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebug() const {return bShouldDrawDebug; }

	//给玩家自身施加一个速度进行Push
	void PushSelf(const FVector& PushVel);

	//获得AvatarCharacter，即Push对象
	ACharacter* GetOwningAvatarCharacter();

	//给目标施加一个Push速度
	void PushTarget(AActor*Target,const FVector& PushVel);


	void ApplyGameplayEffectToHitResultActor(const FHitResult HitResult,TSubclassOf<UGameplayEffect> GameplayEffect,int Level);
	
private:
	UPROPERTY(EditDefaultsOnly,Category="Debug")
	bool bShouldDrawDebug=false;

	UPROPERTY()
	ACharacter* AvatarCharacter;
};
