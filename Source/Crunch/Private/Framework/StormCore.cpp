// Fill out your copyright notice in the Description page of Project Settings.


#include "StormCore.h"
#include "Components/DecalComponent.h"
#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AStormCore::AStormCore()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InfluenceRange=CreateDefaultSubobject<USphereComponent>("Influence Range");
	InfluenceRange->SetupAttachment(GetRootComponent());

	InfluenceRange->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::NewInfluencerInRange);
	InfluenceRange->OnComponentEndOverlap.AddDynamic(this,&ThisClass::InfluencerOutRange);

	ViewCam=CreateDefaultSubobject<UCameraComponent>("View Camera");
	ViewCam->SetupAttachment(GetRootComponent());
	
	GroundDecalComponent=CreateDefaultSubobject<UDecalComponent>("Ground Decal");
	GroundDecalComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AStormCore::BeginPlay()
{
	Super::BeginPlay();
	
}

void AStormCore::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	OwnerAIC=Cast<AAIController>(NewController);
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

void AStormCore::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName=PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AStormCore,InfluenceRadius))
	{
		InfluenceRange->SetSphereRadius( InfluenceRadius);
		const FVector DecalSize=GroundDecalComponent->DecalSize;
		GroundDecalComponent->DecalSize=FVector{DecalSize.X,InfluenceRadius,InfluenceRadius};
	}
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
		const float TeamOffset=TeamOneInfluencerCount-TeamTwoInfluencerCount;
		const float TeamTotal=TeamOneInfluencerCount+TeamTwoInfluencerCount;

		TeamWeight=TeamOffset/TeamTotal;
	}
	UE_LOG(LogTemp,Warning,TEXT("TeamOne Count:%d, TeamTwo Count:%d, Weight:%f"),TeamOneInfluencerCount,TeamTwoInfluencerCount,TeamWeight);

	UpdateGoal();
}

void AStormCore::UpdateGoal()
{
	if (!HasAuthority()) return ;
	if (!OwnerAIC) return ;
	if (!GetCharacterMovement()) return ;

	if (TeamWeight > 0.f)
	{
		OwnerAIC->MoveToActor(TeamOneGoal);
	}
	else
	{
		OwnerAIC->MoveToActor(TeamTwoGoal);
	}

	float Speed=MaxMoveSpeed*FMath::Abs(TeamWeight);
	GetCharacterMovement()->MaxWalkSpeed=Speed;
}
