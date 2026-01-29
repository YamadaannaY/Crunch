// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "ProjectileActor.generated.h"

UCLASS()
class CRUNCH_API AProjectileActor : public AActor
{
	GENERATED_BODY()

public:
	AProjectileActor();
	
	virtual void ShootProjectile(float InSpeed,float InMaxDistance,const AActor* InTarget,FGenericTeamId InTeamId,FGameplayEffectSpecHandle HitEffectHandle);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId);

	virtual FGenericTeamId GetGenericTeamId() const {return TeamId;}
protected:
	virtual void BeginPlay() override;

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

	FGameplayEffectSpecHandle HitEffectSpecHandle;

	FTimerHandle ShootTimeHandle;

	void TravelMaxDistanceReached();
};

