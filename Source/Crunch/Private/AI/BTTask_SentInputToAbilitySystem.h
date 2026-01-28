// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GAS/CGameplayAbilitiesType.h"
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SentInputToAbilitySystem.generated.h"


/**
 *  BTTask 实现BT通过InputID为AI触发GA
 */
UCLASS()
class UBTTask_SentInputToAbilitySystem : public UBTTaskNode
{
	GENERATED_BODY()
public:
	//节点只执行一次逻辑
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	//要触发GA的ID
	UPROPERTY(EditAnywhere,Category="Ability")
	ECAbilityInputID InputID;
};
