#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Components/SphereComponent.h"
#include "ATargetActor_Around.generated.h"

UCLASS()
class CRUNCH_API AATargetActor_Around : public AGameplayAbilityTargetActor,public  IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AATargetActor_Around();

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	FORCEINLINE virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//利用GA自定义的数据配置TA
	void ConfigureAroundActor(float DetectionRadius,const FGenericTeamId TeamId , const FGameplayTag& InLocalGameplayCueTag);

private:
	UPROPERTY(VisibleDefaultsOnly,Category="Components")
	USceneComponent* RootComp;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Components")
	USphereComponent* AroundSphereComponent;

	//即使Configure在客户端服务端都调用，Rep还是必要的，这能保证服务端的值才是权威值，一旦出现不同以服务端为准
	
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;
	
	UPROPERTY(ReplicatedUsing= OnRep_TargetDetectionRadiusReplicated)
	float TargetDetectionRadius;

	UPROPERTY(Replicated)
	FGameplayTag LocalGameplayCueTag;

	UFUNCTION()
	void OnRep_TargetDetectionRadiusReplicated();

	UFUNCTION()
	void ActorInDetectionRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
};
