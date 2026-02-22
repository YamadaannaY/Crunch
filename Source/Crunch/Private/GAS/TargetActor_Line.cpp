#include "TargetActor_Line.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Crunch/Crunch.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"

ATargetActor_Line::ATargetActor_Line() :AvatarActor(nullptr)
{
	//赋予位置信息
	RootComp=CreateDefaultSubobject<USceneComponent>("Root Comp");
	SetRootComponent(RootComp);

	//Sphere组件用于进行CollisionOverlap判断
	TargetEndDetectionSphere=CreateDefaultSubobject<USphereComponent>("Target End Detection Sphere");
	TargetEndDetectionSphere->SetupAttachment(RootComp);
	TargetEndDetectionSphere->SetCollisionResponseToChannel(ECC_SpringArm,ECR_Ignore);

	//VFX，Sphere的视觉表现
	LazerVFX=CreateDefaultSubobject<UNiagaraComponent>("Lazer VFX");
	LazerVFX->SetupAttachment(RootComp);

	//BaseSetting
	PrimaryActorTick.bCanEverTick = true;
	bReplicates=true;
	ShouldProduceTargetDataOnServer=true;
}

void ATargetActor_Line::ConfigureTargetSetting(float NewTargetRange, float NewDetectionCylinderRadius,
	float NewTargetInterval, FGenericTeamId OwnerTeamId, bool bShouldDrawDebug)
{
	TargetRange = NewTargetRange;	
	DetectionCylinderRadius = NewDetectionCylinderRadius;
	TargetingInterval=NewTargetInterval;
	bDrawDebug=bShouldDrawDebug;

	SetGenericTeamId(OwnerTeamId);
}

void ATargetActor_Line::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamId=NewTeamID;
}

void ATargetActor_Line::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATargetActor_Line,TeamId);
	DOREPLIFETIME(ATargetActor_Line,TargetRange);
	DOREPLIFETIME(ATargetActor_Line,AvatarActor);
	DOREPLIFETIME(ATargetActor_Line,DetectionCylinderRadius);
	
}

void ATargetActor_Line::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	if (!OwningAbility) return ;

	AvatarActor=OwningAbility->GetAvatarActorFromActorInfo();
	
	if (HasAuthority())
	{
		//进行TargetActor的查询
		GetWorldTimerManager().SetTimer(PeriodicalTargetingTimerHandle,this,&ATargetActor_Line::DoTargetCheckReport,TargetingInterval,true);
	}
}

void ATargetActor_Line::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTargetTrace();
}

void ATargetActor_Line::BeginDestroy()
{
	if (GetWorld() && PeriodicalTargetingTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(PeriodicalTargetingTimerHandle);
	}
	Super::BeginDestroy();
}

void ATargetActor_Line::DoTargetCheckReport()
{
	if (!HasAuthority()) return ;

	TSet<AActor*> OverlappingActorSet;
	TargetEndDetectionSphere->GetOverlappingActors(OverlappingActorSet);

	//ActorArray要求数据类型为弱指针数组
	TArray<TWeakObjectPtr<AActor>> OverlappingActors;
	for (AActor* OverlappingActor : OverlappingActorSet)
	{
		if (ShouldReportActorAsTarget(OverlappingActor))
		{
			OverlappingActors.Add(OverlappingActor);
		}
	}

	FGameplayAbilityTargetDataHandle TargetDataHandle;

	FGameplayAbilityTargetData_ActorArray* ActorArray=new FGameplayAbilityTargetData_ActorArray;
	ActorArray->SetActors(OverlappingActors);
	TargetDataHandle.Add(ActorArray);

	//触发ValidData回调
	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}

void ATargetActor_Line::UpdateTargetTrace()
{
	FVector ViewLocation=GetActorLocation();
	FRotator ViewRotation=GetActorRotation();

	if (AvatarActor)
	{
		AvatarActor->GetActorEyesViewPoint(ViewLocation,ViewRotation);
	}

	//这里的思路是：EndPoint是摄像机的视角延伸到无限远的位置，但是由于AimStat使得摄像机相对角色有偏移，所以调用函数计算得到一个Rot向量,将ActorMesh的朝向修改为这个Rot
	FVector LookEndPoint=ViewLocation + ViewRotation.Vector()* 100000;
	FRotator LookRotation=UKismetMathLibrary::FindLookAtRotation(GetActorLocation(),LookEndPoint);
	SetActorRotation(LookRotation);

	const FVector SweepEndLocation=GetActorLocation()+LookRotation.Vector() * TargetRange;

	TArray<FHitResult> HitResults;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);
	QueryParams.AddIgnoredActor(this);
	//允许游戏运行时在世界中查找运行时实例
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor->GetName().Contains("StormCore"))
		{
			QueryParams.AddIgnoredActor(Actor);
		}
	}

	FCollisionResponseParams ResponseParams(ECR_Overlap);
	
	GetWorld()->SweepMultiByChannel(HitResults,GetActorLocation(),SweepEndLocation,FQuat::Identity,
		ECC_WorldDynamic,FCollisionShape::MakeSphere(DetectionCylinderRadius),QueryParams,ResponseParams);

	FVector LineEndLocation= SweepEndLocation;
	float LineLength=TargetRange;

	for (FHitResult& HitResult : HitResults)
	{
		if (HitResult.GetActor())
		{
			if (GetTeamAttitudeTowards(*HitResult.GetActor()) != ETeamAttitude::Friendly)
			{
				LineEndLocation= HitResult.ImpactPoint;
				LineLength=FVector::Distance(GetActorLocation(),LineEndLocation);
				//不具备穿透性
				break;
			}
		}
	}

	//对EndLoc位置的Actors进行Collision检测
	TargetEndDetectionSphere->SetWorldLocation(LineEndLocation);
	
	if (LazerVFX)
	{
		//设置Length长度，需要进两位的单位换算，因此除以100
		LazerVFX->SetVariableFloat(LazerVFXLengthParamName,LineLength/100);
	}
}

bool ATargetActor_Line::ShouldReportActorAsTarget(const AActor* ActorToCheck) const
{
	if (!ActorToCheck) return false;
	if (ActorToCheck == AvatarActor) return false;
	if (ActorToCheck == this) return false;
	if (GetTeamAttitudeTowards(*ActorToCheck) == ETeamAttitude::Friendly) return false;

	return true;
}
