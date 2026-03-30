//用TA进行范围Overlap检测记录到TargetDataHandle，并添加绑定添加一个VFX牵引特效显示

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

	//配置黑洞变量
	void ConfigureBlackhole(float InBlackholeRange,float InPullSpeed,float InBlackholeDuration,const FGenericTeamId& InTeamId);
	
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override;
	
	virtual FGenericTeamId GetGenericTeamId() const override {return TeamId; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void Tick(float DeltaTime) override;

	//TargetActor创建时调用
	virtual void StartTargeting(UGameplayAbility* Ability) override;

	//Confirm
	virtual void ConfirmTargetingAndContinue() override;

	//Cancel
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

	UFUNCTION()
	void OnRep_BlackholeRange();

	//SphereComponent的BeginOverlap回调
	UFUNCTION()
	void ActorInBlackholeRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//SphereComponent的EndOverlap回调
	UFUNCTION()
	void ActorLeftBlackholeRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//BeginOverlap检测到Target时创建VFX组件并添加到Map中
	void TryAddTarget(AActor* OtherTarget);

	//EndOverlap时对Target的VFX组件销毁并从Map中移除
	void RemoveTarget(AActor* OtherTarget);

	//存储所有Overlap对象，销毁Niagara组件
	void StopBlackhole();
};
