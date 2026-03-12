#include "CAIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/CCharacter.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ACAIController::ACAIController()
{
	AIPerceptionComponent=CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");
	SightConfig=CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

	SightConfig->DetectionByAffiliation.bDetectEnemies=true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies=false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals=false;
	
	SightConfig->SightRadius=1000.f;
	SightConfig->LoseSightRadius=1200.f;
	//超过LoseSight开始计时
	SightConfig->SetMaxAge(5.f);
	SightConfig->PeripheralVisionAngleDegrees=180.f;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this,&ThisClass::TargetPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this,&ThisClass::TargetGetForgotten);
}

void ACAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	IGenericTeamAgentInterface* PawnTeamInterface=Cast<IGenericTeamAgentInterface>(InPawn);
	if (PawnTeamInterface)
	{
		SetGenericTeamId(PawnTeamInterface->GetGenericTeamId());

		//刷新Senses
		ClearAndDisabledAllSenses();
		EnableAllSenses();
	}

	//监听DeadTag，根据Tag状态添加或者清理所有Sense
	UAbilitySystemComponent* PawnASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InPawn);
	if (PawnASC)
	{
		PawnASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this,&ThisClass::PawnDeadTagUpdated);
		PawnASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatTag()).AddUObject(this,&ThisClass::PawnStunTagUpdated);
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

	//AI失去对目标的感知
	else
	{
		//判断目标失去感知的原因是否是因为死亡，如果是就直接忘记目标
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
		//null
		BlackboardComponent->ClearValue(TargetBlackboardKeyName);
	}
}

AActor* ACAIController::GetNextPerceivedActor() const
{
	//存储所有当前感知到的敌对Actor(包括MaxAge期间的)并返回第一个，如果没有返回null，此时Target为None
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

void ACAIController::ForgetActorIfDead(AActor* ActorToForget) const 
{
	//获得ASC，通过ASC判断是否含有DeadTag
	const UAbilitySystemComponent* ActorASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToForget);
	if (!ActorASC) return ;

	if (ActorASC->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag()))
	{
		//这个Container保存了所有AI当前感知的Actor和他们最新的Stimuli（sight、hear）
		for (UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iter=AIPerceptionComponent->GetPerceptualDataIterator();Iter;++Iter)
		{
			if (Iter->Key!=ActorToForget)
			{
				continue;
			}
			
			//遍历其所有Stimuli,设置其Age直接为最大
			for (FAIStimulus& Stimulus : Iter->Value.LastSensedStimuli)
			{
				Stimulus.SetStimulusAge(TNumericLimits<float>::Max());
			}
		}
	}
}

void ACAIController::ClearAndDisabledAllSenses()
{
	//清空当前AI的一切记忆并让所有感知到期。 这个函数是一次性的，下一帧还是会重新感知到。
	AIPerceptionComponent->AgeStimuli(TNumericLimits<float>::Max());

	//关闭所有感知
	for (auto SenseConfigIt=AIPerceptionComponent->GetSensesConfigIterator();SenseConfigIt;++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(),false);
	}
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->ClearValue(TargetBlackboardKeyName);
	}
}

void ACAIController::EnableAllSenses() const
{
	for (auto SenseConfigIt=AIPerceptionComponent->GetSensesConfigIterator();SenseConfigIt;++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(),true);
	}
}

void ACAIController::PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (Count!=0)
	{
		GetBrainComponent()->StopLogic("Dead");
		ClearAndDisabledAllSenses();
		bIsPawnDead=true;
	}
	else
	{
		GetBrainComponent()->StartLogic();
		EnableAllSenses();
		bIsPawnDead=false;
	}
}

void ACAIController::PawnStunTagUpdated(const FGameplayTag Tag, int32 Count) const 
{
	if (bIsPawnDead) return ;

	if (Count!=0)
	{
		GetBrainComponent()->StopLogic("Stun");
	}
	else
	{
		GetBrainComponent()->StartLogic();
	}
}

