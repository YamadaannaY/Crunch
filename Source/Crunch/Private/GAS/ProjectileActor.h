// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "ProjectileActor.generated.h"

UCLASS()
class CRUNCH_API AProjectileActor : public AActor,public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AProjectileActor();
	
	virtual void ShootProjectile(float InSpeed,float InMaxDistance,const AActor* InTarget,FGenericTeamId InTeamId,FGameplayEffectSpecHandle HitEffectHandle);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;

	virtual FGenericTeamId GetGenericTeamId() const override {return TeamId;}

	//只在碰撞发生的一端被调用，由于设计上Shoot的碰撞建立在Tick修改客户端服务器上Projectile的位置实现，所以两端都会在Overlap时调用此函数
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

public:
	virtual void Tick(float DeltaTime) override;

	void TraveMaxDistanceReached();
private:
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	UPROPERTY(Replicated)
	FVector MoveDir;
	
	UPROPERTY(Replicated)
	float ProjectileSpeed;

	UPROPERTY()
	const AActor* Target;

	UPROPERTY(EditDefaultsOnly,Category="Gameplay Cue")
	FGameplayTag HitGameplayCueTag;

	FVector CacheTargetLocation;

	FTimerHandle ShootTimeHandle;

	FGameplayEffectSpecHandle HitEffectSpecHandle;

private:
	void TravelMaxDistanceReached();

	void SendLocalGameplayCue(AActor* CueTargetActor,const FHitResult& HitResult);
};

