// Fill out your copyright notice in the Description page of Project Settings.


#include "CAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/CCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

// Sets default values
ACAIController::ACAIController()
{
	AIPerceptionComponent=CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");
	SightConfig=CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

	SightConfig->DetectionByAffiliation.bDetectEnemies=true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies=false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals=false;
	SightConfig->SightRadius=1000.f;
	SightConfig->LoseSightRadius=1200.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->PeripheralVisionAngleDegrees=180.f;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this,&ThisClass::TargetPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this,&ThisClass::TargetGetForgotten);
}

void ACAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetGenericTeamId(FGenericTeamId(0));

	IGenericTeamAgentInterface* PawnTeamInterface=Cast<IGenericTeamAgentInterface>(InPawn);
	if (PawnTeamInterface)
	{
		PawnTeamInterface->SetGenericTeamId(GetGenericTeamId());
	}
}

void ACAIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(BehaviorTree);
}

void ACAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	//如果当前目标为空并且成功感知到新目标，则更新目标，否则不更新，即使有新目标也继续锁定旧目标
	if (!GetCurrentTarget())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			SetCurrenTarget(TargetActor);
		}
	}
}

void ACAIController::TargetGetForgotten(AActor* ForgottenActor)
{
	if (!ForgottenActor) return ;

	//如果当前Target正好是ForgotActor则说明应该换取目标，尝试获取其他Target值
	if (GetCurrentTarget()==ForgottenActor)
	{
		SetCurrenTarget(GetNextPerceivedActor());
	}
}
      
const UObject* ACAIController::GetCurrentTarget() const
{
	//获取当前TargetValue
	const UBlackboardComponent* BlackboardComponent=GetBlackboardComponent();
	if (BlackboardComponent)
	{
		return GetBlackboardComponent()->GetValueAsObject(TargetBlackboardKeyName);
	}
	return nullptr;
}

void ACAIController::SetCurrenTarget(AActor* NewTarget)
{
	//将传入参数作为Target的值，如果传入参数为空则将ClearValue
	UBlackboardComponent* BlackboardComponent=GetBlackboardComponent();
	if (!BlackboardComponent) return;
	
	if (NewTarget)
	{
		BlackboardComponent->SetValueAsObject(TargetBlackboardKeyName, NewTarget);
	}
	else
	{
		BlackboardComponent->ClearValue(TargetBlackboardKeyName);
	}
}

AActor* ACAIController::GetNextPerceivedActor() const
{
	//存储所有感知到的敌对Actor并返回第一个，如果没有返回null，此时Targte为None
	if (PerceptionComponent)
	{
		TArray<AActor*> Actors;
		AIPerceptionComponent->GetPerceivedHostileActors(Actors);

		if (Actors.Num() !=0)
		{
			return Actors[0];
		}
	}
	return nullptr;
}

