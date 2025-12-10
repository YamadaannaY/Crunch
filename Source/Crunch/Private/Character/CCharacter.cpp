// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"
#include "net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Widgets/OverHeadStatsGauge.h"

// Sets default values
ACCharacter::ACCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	CAbilitySystemComponent=CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	CAttributeSet=CreateDefaultSubobject<UCAttributeSet>("CAttribute Set");

	OverHeadWidgetComponent=CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());

	BindGASChangeDelegates();

	PerceptionStimuliSourceComponent=CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception StimuliSource Component");

}

void ACCharacter::ServerSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this,this);
	
	//在服务端对属性初始化,GA初始化
	CAbilitySystemComponent->ApplyInitialEffects();
	CAbilitySystemComponent->GiveInitialAbilities();
}

void ACCharacter::ClientSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this,this);
}

bool ACCharacter::IsLocallyControlledByPlayer()
{
	//判断LocalController从而判
	return GetController() && GetController()->IsLocalPlayerController();
}

void ACCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
}

void ACCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCharacter,TeamID);
}

// Called when the game starts or when spawned ， it will be  called both in all client and server
void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	//对于每个客户端中每个Character都要渲染一次OverHeadUI
	ConfigureOverHeadStatusWidget();
	MeshRelativeTransform=GetMesh()->GetRelativeTransform();

	//为刺激源组件添加视觉刺激，即能够触发AI的Sense_Sight
	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
}

// Called every frame
void ACCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* ACCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
}

void ACCharacter::BindGASChangeDelegates()
{
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this,&ThisClass::DeadTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatTag()).AddUObject(this,&ThisClass::StunTagUpdated);
	}
}

void ACCharacter::DeadTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
	{
		StartDeathSequence();
	}
	else
	{
		Respawn();
	}
}

void ACCharacter::StunTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;

	if (NewCount != 0)
	{
		OnStun();
		PlayAnimMontage(StunMontage);
	}
	else
	{
		OnRecoveryFromStun();
		StopAnimMontage(StunMontage);
	}
}

void ACCharacter::ConfigureOverHeadStatusWidget()
{
	if (!OverHeadWidgetComponent) return ;

	//本地玩家不需要Overhead UI
	if (IsLocallyControlledByPlayer())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		return;
	}
	//非本地玩家
	UOverHeadStatsGauge* OverHeadStatsGauge=Cast<UOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	if (OverHeadStatsGauge)
	{
		//属性变化时保持动态更新
		OverHeadStatsGauge->ConfigureWithASC(GetAbilitySystemComponent());
		
		OverHeadWidgetComponent->SetHiddenInGame(false);

		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
		
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityUpdateTimerHandle,this,&ACCharacter::UpdateHeadGaugeVisibility,HeadStatGaugeVisibilityUpdateGap,true);
	}
}

void ACCharacter::UpdateHeadGaugeVisibility()
{
	//这个客户端的LocalPlayer，即UI渲染对象
	APawn* LocalPlayerPawn=UGameplayStatics::GetPlayerPawn(this,0);
	
	if(LocalPlayerPawn)
	{
		//当前Character与本地Pawn的距离差值平方
		float DistSquared=FVector::DistSquared(GetActorLocation(),LocalPlayerPawn->GetActorLocation());
		//决定是否显示UI
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared>HeadStatGaugeVisibilityRangeSquared);
	}
}

void ACCharacter::SetStatusGaugeEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		ConfigureOverHeadStatusWidget();
		OverHeadWidgetComponent->SetVisibility(true);
	}
	else
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		OverHeadWidgetComponent->SetVisibility(false);
	}
}

bool ACCharacter::IsDead() const
{
	return GetAbilitySystemComponent()->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag());

}

void ACCharacter::ReSpawnImmediative()
{
	if (HasAuthority())
	{
		GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UCAbilitySystemStatics::GetDeadStatTag()));
	}
}

void ACCharacter::DeathMontageFinished()
{
	//BUG修复：由于池化ReSpawn机制，可能在DeathMontage还没有播放完就重生了，此时RagDoll还是会在Duration之后触发，需要明确
	//触发条件在DeathStat下
	if (IsDead())
	{
		SetRagDollEnabled(true);
	}
}

void ACCharacter::SetRagDollEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		GetMesh()->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

void ACCharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		const float MontageDuration=PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTImerHandle,this,&ThisClass::DeathMontageFinished,MontageDuration+DeathMontageFinishTimeShift);
	}
}

void ACCharacter::StartDeathSequence()
{
	OnDead();

	//即使DeadTag被添加，正在执行的GA也不会结束，需要Cancel所有GA
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->CancelAllAbilities();
	}
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	SetAIPerceptionStimuliSourceEnabled(false);
}

void ACCharacter::Respawn()
{
	OnRespawn();
	SetAIPerceptionStimuliSourceEnabled(true);
	SetRagDollEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	//用于让DeathMontage BlendOut
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

	//重新复活在最开始生成的位置
	if (HasAuthority() && GetController())
	{
		//弱指针保证PlayerStart不会悬空
		TWeakObjectPtr<AActor> StartSpot=GetController()->StartSpot;

		if (StartSpot.IsValid())
		{
			SetActorTransform(StartSpot->GetActorTransform());
		}
	}

	//回满状态
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->ApplyFullStatsEffect();
	}
}

void ACCharacter::OnDead()
{
}

void ACCharacter::OnRespawn()
{
}

void ACCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID=NewTeamID;
}

FGenericTeamId ACCharacter::GetGenericTeamId() const
{
	return TeamID;
}

void ACCharacter::OnRep_TeamID()
{
	//override in AI class
}

void ACCharacter::SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled)
{
	if (!PerceptionStimuliSourceComponent) return ;

	//是否将刺激源组件添加到感知系统，由于将感知Pawn设置为false，这里可以进行自主判断
	if (bIsEnabled)
	{
		PerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
	else
	{ 
		PerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}

void ACCharacter::OnStun()
{
	//override in character class 
}

void ACCharacter::OnRecoveryFromStun()
{
	//override in character class 
}
