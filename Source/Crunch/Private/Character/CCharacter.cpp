// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"
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

// Called when the game starts or when spawned ， it will be  called both in all client and server
void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	//对于每个客户端中每个Character都要渲染一次OverHeadUI
	ConfigureOverHeadStatusWidget();
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

void ACCharacter::StartDeathSequence()
{
	UE_LOG(LogTemp,Warning,TEXT("Dead"));
}

void ACCharacter::Respawn()
{
	UE_LOG(LogTemp,Warning,TEXT("Respawn"));
}

