// Fill out your copyright notice in the Description page of Project Settings.


#include "CAIController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/CCharacter.h"
#include "GAS/UCAbilitySystemStatics.h"
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

	//监听DeadTag，根据Tag状态添加或者清理所有Sense
	UAbilitySystemComponent* PawnASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InPawn);
	if (PawnASC)
	{
		PawnASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this,&ThisClass::PawnDeadTagUpdated);
	}
}

void ACAIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(BehaviorTree);
}

void ACAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	//如果当前目标为空并且成功感知到目标，则更新目标，否则不更新，即使有新目标也继续锁定旧目标
	if (Stimulus.WasSuccessfullySensed())
	{
		if(!GetCurrentTarget())
			{
				SetCurrenTarget(TargetActor);
			}
	}
	else
	{
		//判断目标是否死亡，如果是就直接忘记目标
		ForgetActorIfDead(TargetActor);
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

void ACAIController::ForgetActorIfDead(AActor* ActorToForget)
{
	//获得ASC，通过ASC判断是否含有DeadTag
	const UAbilitySystemComponent* ActorASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToForget);
	if (!ActorASC) return ;

	if (ActorASC->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag()))
	{
		//这个Container保存了所有AI感知过的Actor和他们最新的Stimuli（sight、hear、etc）
		for (UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iter=AIPerceptionComponent->GetPerceptualDataIterator();Iter;++Iter)
		{
			//找到当前需要遗忘的对象
			if (Iter->Key!=ActorToForget)
			{
				continue;
			}
			//遍历其所有Stimuli,设置其Age直接为最大
			for (FAIStimulus& Stimulus : Iter->Value.LastSensedStimuli)
			{
				//把这个刺激的“存在年龄”设为无限大，让AI认为它已经过期到不能再过期。
				Stimulus.SetStimulusAge(TNumericLimits<float>::Max());
			}
		}
	}
}

void ACAIController::ClearAndDisabledAllSenses()
{
	AIPerceptionComponent->AgeStimuli(TNumericLimits<float>::Max());

	for (auto SenseConfigIt=AIPerceptionComponent->GetSensesConfigIterator();SenseConfigIt;++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(),false);
	}
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->ClearValue(TargetBlackboardKeyName);
	}
}

void ACAIController::EnableAllSenses()
{
	for (auto SenseConfigIt=AIPerceptionComponent->GetSensesConfigIterator();SenseConfigIt;++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(),true);
	}
}

void ACAIController::PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count)
{
	//根据Tag调用两个Sense操控函数
	if (Count!=0)
	{
		GetBrainComponent()->StopLogic("Dead");
		ClearAndDisabledAllSenses();
	}
	else
	{
		GetBrainComponent()->StartLogic();
		EnableAllSenses();
	}
}

