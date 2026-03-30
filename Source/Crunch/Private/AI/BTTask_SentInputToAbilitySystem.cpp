#include "AI/BTTask_SentInputToAbilitySystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

EBTNodeResult::Type UBTTask_SentInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* OwnerAIC = OwnerComp.GetAIOwner();
	if (OwnerAIC == nullptr) return EBTNodeResult::Failed;

	if (OwnerAIC)
	{
		UAbilitySystemComponent* OwnerASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIC->GetPawn());
		if (OwnerASC)
		{
			//ASC已经为InputID配置好GA并Give,直接Press即可触发
			OwnerASC->PressInputID((int32)InputID);
			
			return EBTNodeResult::Succeeded;
		}
	}
	
	return EBTNodeResult::Failed;
}
