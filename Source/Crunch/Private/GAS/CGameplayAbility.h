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
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	//获取瞄准方向范围内具有的目标
	AActor* GetAimTarget(float AimDistance,ETeamAttitude::Type TeamAttitude) const ;
	
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

	//对单施加一个Push速度
	void PushTarget(AActor*Target,const FVector& PushVel);
	//对群
	void PushTargets(const TArray<AActor*>& Targets,const FVector PushVel);
	
	//对TargetData所有Actors
	void PushTarget(const FGameplayAbilityTargetDataHandle& TargetDataHandle,const FVector& PushVel);
	
	void ApplyGameplayEffectToHitResultActor(const FHitResult HitResult,TSubclassOf<UGameplayEffect> GameplayEffect,int Level);

	void SendLocalGameplayEvent(const FGameplayTag& EventTag,const FGameplayEventData& EventData);

	//AnimInstance播放montage，在客户端环境播放
	void PlayMontageLocally(UAnimMontage* MontageToPlay);

	//在当前Section播放完毕后暂停Montage
	void StopMontageAfterCurrentSection(UAnimMontage* MontageToStop);

	//获得AvaActor的Id
	FGenericTeamId GetOwnerTeamId() const;

	//判断TeamAttitude关系
	bool IsActorTeamAttitudeIs(const AActor* OtherActor,ETeamAttitude::Type TeamAttitude) const ;
private:
	UPROPERTY(EditDefaultsOnly,Category="Debug")
	bool bShouldDrawDebug=false;

	UPROPERTY()
	ACharacter* AvatarCharacter;
};
