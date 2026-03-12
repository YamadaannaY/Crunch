//为Minion配置的AIController，主要对Sense、黑板等进行逻辑配置

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CAIController.generated.h"

struct FGameplayTag;
struct FAIStimulus;

/**
 * 
 */

UCLASS()
class CRUNCH_API ACAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACAIController();
	
	//在Pawn的PossessedBy函数调用之后调用，处理AI行为和控制逻辑，且同步PawnID
	virtual void OnPossess(APawn* InPawn) override;
	
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly,Category="AI Behavior")
	FName TargetBlackboardKeyName="Target" ;
	
	UPROPERTY(EditDefaultsOnly,Category="AI Behavior")
	 UBehaviorTree* BehaviorTree;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Perception")
	UAIPerceptionComponent* AIPerceptionComponent;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Perception")
	class UAISenseConfig_Sight* SightConfig;

	bool bIsPawnDead;
	
	//PerceptionUpdate的回调，对新感知对象进行逻辑处理
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor,FAIStimulus Stimulus);

	//OnTargetPerceptionForgotten的回调，MaxAge结束时才改变当前目标，即不立刻忘记，更加符合逻辑
	UFUNCTION()
	void TargetGetForgotten(AActor* ForgottenActor);

	//获得当前Target
	const UObject* GetCurrentTarget() const ;

	//设置当前Target
	void SetCurrenTarget(AActor* NewTarget);

	//返回当前感知到敌对Actor数组的第一个对象
	AActor* GetNextPerceivedActor() const;

	//虽然Character类中在Dead状态设置取消AI感知注册，但是MaxAge仍然作用，需要立刻忘记Actor
	void ForgetActorIfDead(AActor* ActorToForget) const ;

	//DeadTag添加时调用，清除所有感知
	void ClearAndDisabledAllSenses();
	//DeadTag被移除时调用，重新开启所有感知
	void EnableAllSenses() const ;

	//监听AI的DeadTag
	void PawnDeadTagUpdated(const FGameplayTag Tag,int32 Count);
	
	//监听AI的StunTag
	void PawnStunTagUpdated(const FGameplayTag Tag,int32 Count) const ;
};
