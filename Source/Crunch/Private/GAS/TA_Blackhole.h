// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GenericTeamAgentInterface.h"
#include "TA_Blackhole.generated.h"

class USphereComponent;

UCLASS()
class CRUNCH_API ATA_Blackhole : public AGameplayAbilityTargetActor , public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATA_Blackhole();

	//配置黑洞数据
	void ConfigureBlackhole(float InBlackholeRange,float InPullSpeed,float InBlackholeDuration,const FGenericTeamId& InTeamId);

	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;
	
	virtual FGenericTeamId GetGenericTeamId() const {return TeamId; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void Tick(float DeltaTime) override;

	virtual auto StartTargeting(UGameplayAbility* Ability) -> void override;

	virtual void ConfirmTargetingAndContinue() override;

	virtual void CancelTargeting() override;
	
private:
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	USphereComponent* DetectionSphereComponent;

	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	UParticleSystemComponent* VFXComponent;

	float PullSpeed;
	
	float BlackholeDuration;

	FTimerHandle BlackholeDurationTimerHandle;
	
	UPROPERTY(ReplicatedUsing=OnRep_BlackholeRange)
	float BlackholeRange;

	UPROPERTY(EditDefaultsOnly,Category="VFX")
	FName BlackholeVFXOriginVariableName="Origin";

	UPROPERTY(EditDefaultsOnly,Category="VFX")
	class UNiagaraSystem* BlackholeLinkVFX;

	UPROPERTY()
	TMap<AActor* , class UNiagaraComponent*> ActorsInRangeMap;

	//客户端配置SphereRadius
	UFUNCTION()
	void OnRep_BlackholeRange();

	//SphereComponent的BeginOverlap回调
	UFUNCTION()
	void ActorInBlackholeRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//SphereComponent的EndOverlap回调
	UFUNCTION()
	void ActorLeftBlackholeRanege(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//BeginOverlap检测到Tagret时进行Add
	void TryAddTarget(AActor* OtherTarget);

	//EndOverlap检测到Target时进行Remove
	void RemoveTarget(AActor* OtherTarget);

	//Blackhole持续时间结束时调用函数
	void StopBlackhole();
};
