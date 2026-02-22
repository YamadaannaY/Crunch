#include "ATargetActor_Around.h"
#include "UCAbilitySystemStatics.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"

AATargetActor_Around::AATargetActor_Around()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	ShouldProduceTargetDataOnServer=true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	
	AroundSphereComponent = CreateDefaultSubobject<USphereComponent>("Around Sphere Component");
	AroundSphereComponent->SetupAttachment(RootComp);
	AroundSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	AroundSphereComponent->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	AroundSphereComponent->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::ActorInDetectionRange);
}

void AATargetActor_Around::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID=NewTeamID;
}

void AATargetActor_Around::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AATargetActor_Around,TeamID);
	DOREPLIFETIME(AATargetActor_Around,LocalGameplayCueTag);
}

void AATargetActor_Around::ConfigureAroundActor(float DetectionRadius, const FGenericTeamId TeamId,
                                                const FGameplayTag& InLocalGameplayCueTag)
{
	AroundSphereComponent->SetSphereRadius(DetectionRadius);
	LocalGameplayCueTag=InLocalGameplayCueTag;
	SetGenericTeamId(TeamId);
}

void AATargetActor_Around::OnRep_TargetDetectionRadiusReplicated()
{
	AroundSphereComponent->SetSphereRadius(TargetDetectionRadius);
}

void AATargetActor_Around::ActorInDetectionRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this ) return ;

	if (OwningAbility)
	{
		AActor* AvatarActor=OwningAbility->GetAvatarActorFromActorInfo();
		if (AvatarActor == OtherActor) return ;
	}

	if (GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile) return ;

	if (HasAuthority())
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle;
		
		FGameplayAbilityTargetData_ActorArray* ActorArray = new FGameplayAbilityTargetData_ActorArray;
		ActorArray->SetActors(TArray<TWeakObjectPtr<AActor>>{OtherActor});
		TargetDataHandle.Add(ActorArray);

		TargetDataReadyDelegate.Broadcast(TargetDataHandle);
	}
	else
	{
		FHitResult HitResult;
		HitResult.ImpactPoint = OtherActor->GetActorLocation();
		HitResult.ImpactNormal = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		UCAbilitySystemStatics::SendLocalGameplayCue(OtherActor, HitResult, LocalGameplayCueTag);
	}
}

