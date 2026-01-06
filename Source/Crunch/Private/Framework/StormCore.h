// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "StormCore.generated.h"

class UCameraComponent;
class USphereComponent;

UCLASS()
class CRUNCH_API AStormCore : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AStormCore();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//在编辑器中进行修改PropertyName对应的变量时立刻在World中实时调整，而不必等到运行游戏，方便观察
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

private:
	UPROPERTY(EditDefaultsOnly,Category="Move")
	float InfluenceRadius=1000.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Move")
	float MaxMoveSpeed=500.f;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Detection")
	USphereComponent* InfluenceRange;

	UPROPERTY(VisibleDefaultsOnly,Category="Detection")
	UDecalComponent* GroundDecalComponent;

	UPROPERTY(VisibleDefaultsOnly,Category="Detection")
	UCameraComponent* ViewCam;
	
	UFUNCTION()
	void NewInfluencerInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void InfluencerOutRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdateTeamWeight();
	void UpdateGoal();

	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamOneGoal;

	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamTwoGoal;
	
	int TeamOneInfluencerCount=0;
	int TeamTwoInfluencerCount=0;

	float TeamWeight =0.f ;

	UPROPERTY()
	class AAIController* OwnerAIC;
};
