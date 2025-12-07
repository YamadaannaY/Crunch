// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_SentInputToAbilitySystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

EBTNodeResult::Type UBTTask_SentInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//获得Controller
	const AAIController* OwnerAIC = OwnerComp.GetAIOwner();
	
	if (OwnerAIC == nullptr) return EBTNodeResult::Failed;

	//获得Controller控制下Pawn的ASC，触发对应ID的GA
	if (OwnerAIC)
	{
		UAbilitySystemComponent* OwnerASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIC->GetPawn());
		if (OwnerASC)
		{
			OwnerASC->PressInputID((int32)InputID);
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
