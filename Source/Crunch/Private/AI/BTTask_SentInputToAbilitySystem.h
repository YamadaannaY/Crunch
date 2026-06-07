#pragma once

#include "GAS/CGameplayAbilitiesType.h"
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SentInputToAbilitySystem.generated.h"


/*
 * 自定义行为树节点，通过InputID触发Minion的GA
 */
UCLASS()
class UBTTask_SentInputToAbilitySystem : public UBTTaskNode
{
	GENERATED_BODY()
public:
	//执行函数
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	//要触发GA对应的InputID
	UPROPERTY(EditAnywhere,Category="Ability")
	ECAbilityInputID InputID;
};
