// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CAIController.generated.h"

struct FAIStimulus;

/**
 * AI
 */

UCLASS()
class CRUNCH_API ACAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACAIController();
	//只在服务端执行
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

private:
	//通过KeyName为对应Key赋值
	UPROPERTY(EditDefaultsOnly,Category="AI Behavior")
	FName TargetBlackboardKeyName="Target" ;
	
	UPROPERTY(EditDefaultsOnly,Category="AI Behavior")
	 UBehaviorTree* BehaviorTree;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Perception")
	UAIPerceptionComponent* AIPerceptionComponent;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Perception")
	class UAISenseConfig_Sight* SightConfig;

	//PerceptionUpdate的回调，对感知对象进行逻辑处理
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor,FAIStimulus Stimulus);

	//OnTargetPerceptionForgotten的回调，MaxAge结束时才改变当前目标，符合现实逻辑
	UFUNCTION()
	void TargetGetForgotten(AActor* ForgottenActor);

	//获得当前Target
	const UObject* GetCurrentTarget() const ;

	//设置当前Target
	void SetCurrenTarget(AActor* NewTarget);

	//返回当前感知到敌对Actor数组的第一个对象
	AActor* GetNextPerceivedActor() const;
};
