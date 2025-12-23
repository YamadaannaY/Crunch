// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CPlayerController.h"
#include "CPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Crunch/DebugHelper.h"
#include "Widgets/GameplayWidget.h"
#include "net/UnrealNetwork.h"

void ACPlayerController::OnPossess(APawn* NewPawn)
{
	//这里判断了当前是服务端的PlayerController
	Super::OnPossess(NewPawn);

	//获取服务端中权威的PlayerCharacter，对其进行初始化。这样做而不是在客户端进行初始化有效防止了作弊
	CPlayerCharacter=Cast<ACPlayerCharacter>(NewPawn);

	if (CPlayerCharacter)
	{
		CPlayerCharacter->ServerSideInit();
		
		//为Character分配ID
		CPlayerCharacter->SetGenericTeamId(TeamID);
	}
}

void ACPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	//这里判断了是在客户端的PlayerController
	Super::AcknowledgePossession(NewPawn);
	
	CPlayerCharacter=Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		CPlayerCharacter->ClientSideInit();
		//在客户端渲染即可，使用复制的值进行UI动态
		SpawnGameplayWidget();
	}
}

void ACPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID=NewTeamID;
}

FGenericTeamId ACPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void ACPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACPlayerController, TeamID);
	
}

void ACPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem=GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (InputSubsystem)
	{
		InputSubsystem->RemoveMappingContext(UIInputMapping);
		InputSubsystem->AddMappingContext(UIInputMapping,1);
	}
	
	UEnhancedInputComponent* EnhancedInputComp=Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(ShopToggleInputAction,ETriggerEvent::Triggered,this,&ThisClass::ToggleShop);
	}
}

void ACPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController()) return;

	//本地Player拥有的视口UI
	GameplayWidget=CreateWidget<UGameplayWidget>(this,GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();

		//配置GA在ListItem中的Widget
		GameplayWidget->ConfigureAbilities(CPlayerCharacter->GetAbilities());
	}
}

void ACPlayerController::ToggleShop()
{
	if (GameplayWidget)
	{
		GameplayWidget->ToggleShop();
	}
}
