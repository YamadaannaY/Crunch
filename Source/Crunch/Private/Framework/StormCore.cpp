// Fill out your copyright notice in the Description page of Project Settings.


#include "StormCore.h"

#include "GenericTeamAgentInterface.h"


// Sets default values
	AStormCore::AStormCore()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InfluenceRange=CreateDefaultSubobject<USphereComponent>("Influence Range");
	InfluenceRange->SetupAttachment(GetRootComponent());

	InfluenceRange->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::NewInfluencerInRange);
	InfluenceRange->OnComponentEndOverlap.AddDynamic(this,&ThisClass::InfluencerOutRange);
}

// Called when the game starts or when spawned
void AStormCore::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStormCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AStormCore::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AStormCore::NewInfluencerInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IGenericTeamAgentInterface* OtherTeamAgentInterface=Cast<IGenericTeamAgentInterface>(OtherActor);
	if (OtherTeamAgentInterface)
	{
		if (OtherTeamAgentInterface->GetGenericTeamId().GetId()==0)
		{
			TeamOneInfluencerCount++;
		}
		else if (OtherTeamAgentInterface->GetGenericTeamId().GetId()==1)
		{
			TeamTwoInfluencerCount++;
		}
		UpdateTeamWeight();
	}
}

void AStormCore::InfluencerOutRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IGenericTeamAgentInterface* OtherTeamAgentInterface=Cast<IGenericTeamAgentInterface>(OtherActor);
	if (OtherTeamAgentInterface)
	{
		if (OtherTeamAgentInterface->GetGenericTeamId().GetId()==0)
		{
			TeamOneInfluencerCount--;
			if (TeamOneInfluencerCount<0)
			{
				TeamOneInfluencerCount=0;
			}
		}
		else if (OtherTeamAgentInterface->GetGenericTeamId().GetId()==1)
		{
			TeamTwoInfluencerCount--;
			if (TeamTwoInfluencerCount<0)
			{
				TeamTwoInfluencerCount=0;
			}
		}
		UpdateTeamWeight();
	}
}

void AStormCore::UpdateTeamWeight()
{
	if (TeamOneInfluencerCount==TeamTwoInfluencerCount)
	{
		TeamWeight=0.f;
	}
	else
	{
		float TeamOffset=TeamOneInfluencerCount-TeamTwoInfluencerCount;
		float TeamTotal=TeamOneInfluencerCount+TeamTwoInfluencerCount;

		TeamWeight=TeamOffset/TeamTotal;
	}

	UE_LOG(LogTemp,Warning,TEXT("TeamOne Count:%d, TeamTwo Count:%d, Weight:%f"),TeamOneInfluencerCount,TeamTwoInfluencerCount,TeamWeight);
}