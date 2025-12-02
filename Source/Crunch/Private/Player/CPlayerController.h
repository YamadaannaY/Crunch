// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	//只在服务端执行，此时执行ServerInit
	virtual void OnPossess(APawn* NewPawn) override;
	
	//只在客户端和监听服务器执行，调用ClientInit
	virtual void AcknowledgePossession(APawn* NewPawn) override;

private:

	//在本地Player的视口内渲染UI
	void SpawnGameplayWidget();
	
	UPROPERTY()
	class ACPlayerCharacter* CPlayerCharacter;

	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<class UGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	UGameplayWidget* GameplayWidget;
};
