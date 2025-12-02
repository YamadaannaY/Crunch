// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CPlayerController.h"
#include "CPlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/GameplayWidget.h"

void ACPlayerController::OnPossess(APawn* NewPawn)
{
	//这里判断了当前是服务端的PlayerController
	Super::OnPossess(NewPawn);

	//获取服务端中权威的PlayerCharacter，对其进行初始化。这样做而不是在客户端进行初始化有效防止了作弊
	CPlayerCharacter=Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		CPlayerCharacter->ServerSideInit();
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

void ACPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController()) return;

	//本地Player拥有的视口UI
	GameplayWidget=CreateWidget<UGameplayWidget>(this,GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
	}
}
