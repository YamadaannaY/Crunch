// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GenericTeamAgentInterface.h"
#include "TargetActor_Line.generated.h"

UCLASS()
class ATargetActor_Line : public AGameplayAbilityTargetActor , public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATargetActor_Line();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//通过WaitTargetData异步节点（Task）在Ability中产生一个TargetActor之后调用，开始对TargetData的配置。
	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void Tick(float DeltaTime) override;
	
	virtual void BeginDestroy() override;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	FORCEINLINE virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	//用于自定义TargetActor_Line类的所有配置参数
	void ConfigureTargetSetting(float NewTargetRange,float NewDetectionCylinderRadius,float NewTargetInterval,FGenericTeamId OwnerTeamId,bool bShouldDrawDebug);
	
private:
	UPROPERTY(Replicated)
	float TargetRange;

	UPROPERTY(Replicated)
	float DetectionCylinderRadius;

	UPROPERTY(Replicated)
	float TargetingInterval;

	UPROPERTY(Replicated)
	FGenericTeamId TeamId;
	
	UPROPERTY(Replicated)
	const AActor* AvatarActor;

	UPROPERTY(EditDefaultsOnly,Category="VFX")
	FName LazerVFXLengthParamName;
	
	UPROPERTY()
	bool bDrawDebug;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	class UNiagaraComponent* LazerVFX;

	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	class USphereComponent* TargetEndDetectionSphere;

	FTimerHandle PeriodicalTargetingTimerHandle;

	//找到所有TargetData应该存储的Actor
	void DoTargetCheckReport();

	//更新Target的末尾落点，从而更新Sphere的轨迹
	void UpdateTargetTrace();

	//对Actors进行过滤
	bool ShouldReportActorAsTarget(const AActor* ActorToCheck) const;
};
