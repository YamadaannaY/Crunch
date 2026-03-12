#include "StormCore.h"
#include "Components/DecalComponent.h"
#include "AIController.h"
#include "Camera/CameraComponent.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ProjectileActor.h"
#include "Player/CPlayerCharacter.h"
#include "net/UnrealNetwork.h"

AStormCore::AStormCore()
{
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

void AStormCore::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AStormCore,CoreToCapture,COND_None,REPNOTIFY_Always)
}

float AStormCore::GetProgress() const
{
	const FVector TeamTwoGoalLoc=TeamTwoGoal->GetActorLocation();
	FVector VectorFromTeamOne=GetActorLocation()- TeamTwoGoalLoc;
	VectorFromTeamOne.Z=0.f;

	//获得TeamOne相对于总Length的比例
	return VectorFromTeamOne.Size()/TravelLength;
}

void AStormCore::BeginPlay()
{
	Super::BeginPlay();
	const FVector TeamOneGoalLoc=TeamOneGoal->GetActorLocation();
	const FVector TeamTwoGoalLoc=TeamTwoGoal->GetActorLocation();

	FVector GoalOffset=TeamOneGoalLoc-TeamTwoGoalLoc;
	GoalOffset.Z=0;

	//两个TeamGoal之间距离的绝对值
	TravelLength=GoalOffset.Length();
}

void AStormCore::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	OwnerAIC=Cast<AAIController>(NewController);
}

void AStormCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CoreToCapture)
	{
		FVector CoreMoveDir=(GetMesh()->GetComponentLocation() - CoreToCapture->GetActorLocation()).GetSafeNormal();
		CoreToCapture->AddActorWorldOffset(CoreMoveDir*CoreCaptureSpeed*DeltaTime);
	}
}

void AStormCore::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName=PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AStormCore,InfluenceRadius))
	{
		InfluenceRange->SetSphereRadius(InfluenceRadius);
		const FVector DecalSize=GroundDecalComponent->DecalSize;
		GroundDecalComponent->DecalSize=FVector{DecalSize.X,InfluenceRadius,InfluenceRadius};
	}
}

void AStormCore::NewInfluencerInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//判断是否抵达目标点
	if (OtherActor==TeamOneGoal)
	{
		GoalReached(0);
	}
	if (OtherActor==TeamTwoGoal)
	{
		GoalReached(1);
	}
	
	IGenericTeamAgentInterface* OtherTeamAgentInterface=Cast<IGenericTeamAgentInterface>(OtherActor);
	if (OtherTeamAgentInterface && !OtherActor->IsA(AProjectileActor::StaticClass()))
	{
		if (OtherTeamAgentInterface->GetGenericTeamId().GetId()==0)
		{
			if (OtherActor->IsA(ACPlayerCharacter::StaticClass()))
			{
				TeamOneInfluencerCount+=4;
			}
			else
			{
				TeamOneInfluencerCount++;
			}
		}
		else if (OtherTeamAgentInterface->GetGenericTeamId().GetId()==1)
		{
			if (OtherActor->IsA(ACPlayerCharacter::StaticClass()))
			{
				TeamTwoInfluencerCount+=4;
			}
			else
			{
				TeamTwoInfluencerCount++;
			}
		}
		UpdateTeamWeight();
	}
}

void AStormCore::InfluencerOutRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IGenericTeamAgentInterface* OtherTeamAgentInterface=Cast<IGenericTeamAgentInterface>(OtherActor);
	if (OtherTeamAgentInterface && !OtherActor->IsA(AProjectileActor::StaticClass()))
	{
		if (OtherTeamAgentInterface->GetGenericTeamId().GetId()==0 )
		{
			if (OtherActor->IsA(ACPlayerCharacter::StaticClass()))
			{
				TeamOneInfluencerCount-=4;
			}
			else
			{
				TeamOneInfluencerCount--;
			}
			
			if (TeamOneInfluencerCount<0)
			{
				TeamOneInfluencerCount=0;
			}
		}
		else if (OtherTeamAgentInterface->GetGenericTeamId().GetId()==1)
		{
			if (OtherActor->IsA(ACPlayerCharacter::StaticClass()))
			{
				TeamTwoInfluencerCount-=4;
			}
			else
			{
				TeamTwoInfluencerCount--;
			}
			
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
	OnTeamInfluenceCountUpdatedDelegate.Broadcast(TeamOneInfluencerCount,TeamTwoInfluencerCount);
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

	UpdateCore();
}

void AStormCore::UpdateCore() const 
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

void AStormCore::CaptureTeamCore()
{
	const float ExpandDuration=GetMesh()->GetAnimInstance()->Montage_Play(ExpandMontage);

	CoreCaptureSpeed=FVector::Distance(GetMesh()->GetComponentLocation(),CoreToCapture->GetActorLocation())/ExpandDuration;

	CoreToCapture->SetActorEnableCollision(false);
	
	GetCharacterMovement()->MaxWalkSpeed=0.f;

	FTimerHandle ExpandTimerHandle;
	GetWorldTimerManager().SetTimer(ExpandTimerHandle,this,&ThisClass::ExpandFinished,ExpandDuration);
}

void AStormCore::ExpandFinished() const 
{
	CoreToCapture->SetActorLocation(GetMesh()->GetComponentLocation());
	CoreToCapture->AttachToComponent(GetMesh(),FAttachmentTransformRules::KeepWorldTransform,"root");

	GetMesh()->GetAnimInstance()->Montage_Play(CaptureMontage);
}

void AStormCore::OnRep_CoreToCapture()
{
	//客户端Capture
	if (CoreToCapture)
	{
		CaptureTeamCore();
	}
}

void AStormCore::GoalReached(int WiningTeam)
{
	OnGoalReachedDelegate.Broadcast(this,WiningTeam);

	//服务端确定CaptureCore
	if (!HasAuthority()) return ;

	MaxMoveSpeed=0.f;
	CoreToCapture=WiningTeam==0 ? TeamTwoCore : TeamOneCore;

	//服务端Capture
	CaptureTeamCore();
}
