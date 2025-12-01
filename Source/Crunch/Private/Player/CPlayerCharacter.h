// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CCharacter.h"
#include "CPlayerCharacter.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class ACPlayerCharacter : public ACCharacter
{
	GENERATED_BODY()
public:
	ACPlayerCharacter();

	//在客户端的Pawn发生改变时或者客户端ClientTravel进入关卡后调用，会启用输入，重新初始化AbilityInput
	virtual void PawnClientRestart() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
private:
	UPROPERTY(VisibleDefaultsOnly,Category="View")
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleDefaultsOnly,Category="View")
	class UCameraComponent* ViewCamera;

	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputAction* JumpInputAction;
	
	UPROPERTY(EditDefaultsOnly,Category="Input")
    UInputAction* LookInputAction;

	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputAction* MoveInputAction;
	
	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputMappingContext* GameplayInputMappingContext;
	
	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleMoveInput(const FInputActionValue& InputActionValue);

	FVector GetLookRightDir() const ;
	FVector GetLookFwdDir() const ;
	FVector GetMoveFwdDir() const ;
};
