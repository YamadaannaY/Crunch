
#include "TA_Blackhole.h"

#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
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

	if (HasAuthority())
	{
		for (TPair<AActor*,UNiagaraComponent*>& TargetPair : ActorsInRangeMap)
		{
			AActor* Target=TargetPair.Key;
			UNiagaraComponent* NiagaraComponent=TargetPair.Value;
			
			FVector PullDir=(GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
			
			Target->SetActorLocation(Target->GetActorLocation() + PullDir*PullSpeed*DeltaTime);

			if (NiagaraComponent)
			{
				NiagaraComponent->SetVariablePosition(BlackholeVFXOriginVariableName,VFXComponent->GetComponentLocation());
			}
		}
	}
}

void ATA_Blackhole::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(BlackholeDurationTimerHandle,this,&ThisClass::StopBlackhole,BlackholeDuration);
	}
}

void ATA_Blackhole::ConfirmTargetingAndContinue()
{
	StopBlackhole();
}

void ATA_Blackhole::CancelTargeting()
{
	StopBlackhole();
	
	Super::CancelTargeting();
}

void ATA_Blackhole::OnRep_BlackholeRange()
{
	DetectionSphereComponent->SetSphereRadius(BlackholeRange);
}

void ATA_Blackhole::ActorInBlackholeRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryAddTarget(OtherActor);
}

void ATA_Blackhole::ActorLeftBlackholeRanege(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	RemoveTarget(OtherActor);
}

void ATA_Blackhole::TryAddTarget(AActor* OtherTarget)
{
	if (!OtherTarget || ActorsInRangeMap.Contains(OtherTarget)) return ;
	if (GetTeamAttitudeTowards(*OtherTarget) != ETeamAttitude::Hostile) return ;

	UNiagaraComponent* NiagaraComponent=nullptr;

	if (BlackholeLinkVFX)
	{
		NiagaraComponent=UNiagaraFunctionLibrary::SpawnSystemAttached(
			BlackholeLinkVFX,
			OtherTarget->GetRootComponent(),
			NAME_None,
			FVector::Zero(),FRotator::ZeroRotator,
			EAttachLocation::Type::KeepRelativeOffset,false);

		//确定LinkVFX的起始位置参数
		if(NiagaraComponent)
		{
			NiagaraComponent->SetVariablePosition(BlackholeVFXOriginVariableName,VFXComponent->GetComponentLocation());
		}
	}

	ActorsInRangeMap.Add(OtherTarget,NiagaraComponent);
}

void ATA_Blackhole::RemoveTarget(AActor* OtherTarget)
{
	if (!OtherTarget) return ;

	if (ActorsInRangeMap.Contains(OtherTarget))
	{
		UNiagaraComponent* VFXComp;
		ActorsInRangeMap.RemoveAndCopyValue(OtherTarget,VFXComp);
		if (IsValid(VFXComp))
		{
			VFXComp->DestroyComponent();
		}
	}
}

void ATA_Blackhole::StopBlackhole()
{
	TArray<TWeakObjectPtr<AActor>> FinalTargets;

	for (TPair<AActor* ,UNiagaraComponent*>& TargetPair : ActorsInRangeMap)
	{
		FinalTargets.Add(TargetPair.Key);

		UNiagaraComponent* NiagaraComponent= TargetPair.Value;
		if (IsValid(NiagaraComponent))
		{
			NiagaraComponent->DestroyComponent();
		}
	}

	//传递TargetActors
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_ActorArray* TargetActorArray = new FGameplayAbilityTargetData_ActorArray();
	TargetActorArray->SetActors(FinalTargets);
	TargetDataHandle.Add(TargetActorArray);

	//传递HitResult
	FGameplayAbilityTargetData_SingleTargetHit* BlowupLocation= new FGameplayAbilityTargetData_SingleTargetHit();
	BlowupLocation->HitResult.ImpactPoint = GetActorLocation();
	TargetDataHandle.Add(BlowupLocation);

	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}