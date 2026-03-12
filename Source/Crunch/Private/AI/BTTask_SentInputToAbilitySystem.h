//行为树中节点，通过InputID触发GA

#pragma once

#include "GAS/CGameplayAbilitiesType.h"
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SentInputToAbilitySystem.generated.h"


/**
 *
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
