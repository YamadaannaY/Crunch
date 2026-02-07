// Fill out your copyright notice in the Description page of Project Settings.


#include "TA_Blackhole.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

ATA_Blackhole::ATA_Blackhole()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates=true;
	ShouldProduceTargetDataOnServer=true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);
	
	DetectionSphereComponent=CreateDefaultSubobject<USphereComponent>("Detection Sphere Component");
	DetectionSphereComponent->SetupAttachment(RootComp);
	DetectionSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DetectionSphereComponent->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::ActorInBlackholeRange);
	DetectionSphereComponent->OnComponentEndOverlap.AddDynamic(this,&ThisClass::ActorLeftBlackholeRanege);

	VFXComponent=CreateDefaultSubobject<UParticleSystemComponent>("VFX Component");
	VFXComponent->SetupAttachment(RootComp);
}

void ATA_Blackhole::ConfigureBlackhole(float InBlackholeRange, float InPullSpeed, float InBlackholeDuration,
	const FGenericTeamId& InTeamId)
{
	PullSpeed=InPullSpeed;
	DetectionSphereComponent->SetSphereRadius(InBlackholeRange);
	SetGenericTeamId(InTeamId);
	BlackholeDuration=InBlackholeDuration;
	BlackholeRange=InBlackholeRange;
}

void ATA_Blackhole::SetGenericTeamId(const FGenericTeamId& InTeamID)
{
	TeamId=InTeamID;
}

void ATA_Blackhole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ATA_Blackhole,TeamId);
	DOREPLIFETIME_CONDITION_NOTIFY(ATA_Blackhole,BlackholeRange,COND_None,REPNOTIFY_Always)
}

void ATA_Blackhole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATA_Blackhole::OnRep_BlackholeRange()
{
	DetectionSphereComponent->SetSphereRadius(BlackholeRange);
}

void ATA_Blackhole::ActorInBlackholeRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void ATA_Blackhole::ActorLeftBlackholeRanege(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}