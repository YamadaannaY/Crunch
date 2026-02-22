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

	//根据Vel方向向量参数对单施加一个Push效果
	void PushTarget(AActor*Target,const FVector& PushVel);
	
	//对群
	void PushTargets(const TArray<AActor*>& Targets,const FVector PushVel);
	
	//对TargetData所有Actors
	void PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle,const FVector& PushVel);

	//从Handle获取Targets根据Loc位置向量参数计算得到方向，施加Push效果
	void PushTargetsFromLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& FromLocation ,float PushSpeed);

	//Targets根据Loc位置向量参数计算得到方向，施加Push效果
	void PushTargetsFromLocation(const TArray<AActor*>& Targets, const FVector& FromLocation , float PushSpeed);

	//对象为Actors，封装Loc为AvatarActor的位置
	void PushTargetsFromOwnerLocation(const TArray<AActor*>& Targets,float PushSpeed);

	//对象为TargetDataHandle，从中取出Actors调用PushTargets
	void PushTargetsFromOwningLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle,float PushSpeed);

	//将GE赋予HitResult中存储的Actor对象
	void ApplyGameplayEffectToHitResultActor(const FHitResult HitResult,TSubclassOf<UGameplayEffect> GameplayEffect,int Level);

	//客户端本地执行Tag对应的GameplayEvent
	void SendLocalGameplayEvent(const FGameplayTag& EventTag,const FGameplayEventData& EventData);

	//AnimInstance播放Montage，在客户端本地播放
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
