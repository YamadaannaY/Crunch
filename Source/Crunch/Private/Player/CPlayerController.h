// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "CPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACPlayerController : public APlayerController ,public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	//只在服务端执行，此时执行ServerInit
	virtual void OnPossess(APawn* NewPawn) override;
	
	//只在客户端和监听服务器执行，调用ClientInit
	virtual void AcknowledgePossession(APawn* NewPawn) override;

	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetupInputComponent() override;
private:

	//在本地Player的视口内渲染UI
	void SpawnGameplayWidget();
	
	UPROPERTY()
	class ACPlayerCharacter* CPlayerCharacter;

	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<class UGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	UGameplayWidget* GameplayWidget;

	//这个TeamID是由位于服务端的ServerPC权威设置的，设置Replicated的目的是把ID同步给ClientPC
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

	UPROPERTY(EditDefaultsOnly,Category="Input")
	class UInputMappingContext* UIInputMapping;

	UPROPERTY(EditDefaultsOnly,Category="Input")
	class UInputAction* ShopToggleInputAction;

	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputAction* ToggleGameplayMenuAction;
	
	UFUNCTION()
	void ToggleShop();

	UFUNCTION()
	void ToggleGameplayMenu();
};
