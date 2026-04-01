#include "ProjectileActor.h"
#include "GenericTeamAgentInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "net/UnrealNetwork.h"

AProjectileActor::AProjectileActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	USceneComponent* RootComp=CreateDefaultSubobject<USceneComponent>("Root Comp");
	SetRootComponent(RootComp);
}

void AProjectileActor::ShootProjectile(float InSpeed, float InMaxDistance, const AActor* InTarget, FGenericTeamId InTeamId,
	FGameplayEffectSpecHandle HitEffectHandle)
{
	Target=InTarget;
	
	ProjectileSpeed=InSpeed;

	SetGenericTeamId(InTeamId);

	if (GetOwner())
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		GetOwner()->GetActorEyesViewPoint(CameraLocation,CameraRotation);
		
		float MaxDistance = InMaxDistance; // 最大射程
		FVector TraceEnd = CameraLocation + CameraRotation.Vector() * MaxDistance;
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this); // 忽略自己

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, QueryParams);

		// 4. 确定目标点
		FVector TargetPoint = bHit ? HitResult.Location : TraceEnd;
		
		MoveDir=(TargetPoint-GetActorLocation()).GetSafeNormal();
	}
	
	HitEffectSpecHandle=HitEffectHandle;

	const float TravelMaxTime=InMaxDistance / InSpeed;
	GetWorldTimerManager().SetTimer(ShootTimeHandle,this,&ThisClass::TravelMaxDistanceReached,TravelMaxTime);
}

void AProjectileActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectileActor,MoveDir);
	DOREPLIFETIME(AProjectileActor,TeamId);
	DOREPLIFETIME(AProjectileActor,ProjectileSpeed)
}

void AProjectileActor::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	TeamId=NewTeamId;
}

void AProjectileActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!OtherActor || OtherActor==GetOwner()) return ;

	if (GetTeamAttitudeTowards(*OtherActor) != ETeamAttitude::Hostile) return ;
	
	UAbilitySystemComponent* OtherASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (OtherASC)
	{
		if (HasAuthority() && HitEffectSpecHandle.IsValid())
		{
			//为被击中者施加GE
			OtherASC->ApplyGameplayEffectSpecToSelf(*HitEffectSpecHandle.Data.Get());
			GetWorldTimerManager().ClearTimer(ShootTimeHandle);
		}

		FHitResult HitResult;
		HitResult.ImpactPoint=GetActorLocation();
		HitResult.ImpactNormal=GetActorForwardVector();

		SendLocalGameplayCue(OtherActor,HitResult);
		
		Destroy();
	}
}

void AProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (HasAuthority())
	{
		if (Target)
		{
			//Rep到客户端
			MoveDir=(Target->GetActorLocation()-GetActorLocation()).GetSafeNormal();
		}
	}
	
	//Server+Client修改方向
	SetActorLocation(GetActorLocation() + MoveDir*ProjectileSpeed*DeltaTime);
}

void AProjectileActor::TravelMaxDistanceReached()
{
	Destroy();
}

void AProjectileActor::SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult) const 
{
	FGameplayCueParameters CueParam;
	CueParam.Location=HitResult.ImpactPoint;
	CueParam.Normal=HitResult.ImpactNormal;

	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(CueTargetActor,HitGameplayCueTag,EGameplayCueEvent::Executed,CueParam);
}
