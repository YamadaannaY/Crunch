// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "StormCore.generated.h"

class UCameraComponent;
class USphereComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGoalReachedDelegate,AActor*,int);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTeamInfluenceCountUpdatedDelegate,int,int );

UCLASS()
class CRUNCH_API AStormCore : public ACharacter
{
	GENERATED_BODY()

public:
	FOnGoalReachedDelegate OnGoalReachedDelegate;
	FOnTeamInfluenceCountUpdatedDelegate OnTeamInfluenceCountUpdatedDelegate;

	AStormCore();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//获取Core相对于两个Gaol之间距离的位置，折算为Progress
	float GetProgress() const ;
protected:
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
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

	UPROPERTY(EditDefaultsOnly,Category="Montage")
	UAnimMontage* ExpandMontage;

	UPROPERTY(EditDefaultsOnly,Category="Montage")
	UAnimMontage* CaptureMontage;
	
	UFUNCTION()
	void NewInfluencerInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void InfluencerOutRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//更新Weight值
	void UpdateTeamWeight();
	//更新Goal的移动方向和速度
	void UpdateGoal();
	
	//到达最终目标位置后，StormCore扩张对TeamCore进行捕捉
	void CaptureCore();
	
	//捕捉完成后收缩
	
	void ExpandFinished();
	
	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamOneGoal;

	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamTwoGoal;
	
	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamOneCore;

	UPROPERTY(EditAnywhere,Category="Team")
	AActor* TeamTwoCore;

	UPROPERTY(ReplicatedUsing=OnRep_CoreToCapture)
	AActor* CoreToCapture;

	float CoreCaptureSpeed;
	float TravelLength=0.f;
	
	UFUNCTION()
	void OnRep_CoreToCapture();

	void GoalReached(int WiningTeam);
	
	int TeamOneInfluencerCount=0;
	int TeamTwoInfluencerCount=0;

	float TeamWeight =0.f ;

	UPROPERTY()
	class AAIController* OwnerAIC;
};
