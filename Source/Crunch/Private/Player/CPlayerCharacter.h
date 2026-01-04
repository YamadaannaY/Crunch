// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Character/CCharacter.h"
#include "GAS/CGameplayAbilitiesType.h"
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

	//只在本地客户端执行，处理用户输入（这也是不在服务端执行的原因，服务端不需要知道输入是什么），只是处理输入后要调用的逻辑
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
private:
	UPROPERTY()
	class UCHeroAttributeSet* HeroAttributesSet;
	
	UPROPERTY(VisibleDefaultsOnly,Category="View")
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleDefaultsOnly,Category="View")
	class UCameraComponent* ViewCamera;

	/*******************	GA	********************************/
private:
	//Aim逻辑重写
	virtual void OnAimStatChanged(bool bIsAiming) override;

	
	/*******************	Input	********************************/

	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputAction* JumpInputAction;
	
	UPROPERTY(EditDefaultsOnly,Category="Input")
    UInputAction* LookInputAction;

	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputAction* LearnAbilityLearnLeaderAction;
	
	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputAction* UseInventoryITemAction;
	
	UPROPERTY(EditDefaultsOnly,Category="Input")
	TMap<ECAbilityInputID, UInputAction*> GameplayAbilityInputAction; 
	
	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputMappingContext* GameplayInputMappingContext;
	
	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void LearnAbilityLeaderDown(const FInputActionValue& InputActionValue);
	void LearnAbilityLeaderUp(const FInputActionValue& InputActionValue);
	void HandleAbilityInput(const FInputActionValue& InputActionValue,ECAbilityInputID InputID);
	void UseInventoryItem(const FInputActionValue& InputActionValue);
	
	void SetInputEnabledFromPlayerController(bool bEnabled);

	FVector GetLookRightDir() const ;
	FVector GetLookFwdDir() const ;
	FVector GetMoveFwdDir() const ;

	bool bIsLearnAbilityLeaderDown=false;

	/******************* Death and Respawn **************************/
	virtual void OnDead() override;
	virtual void OnRespawn() override;

	/******************* Stun **************************/
	virtual void OnStun() override;
	virtual void OnRecoveryFromStun() override;

	/******************* Camera View **************************/
private:
	//Camera偏移值
	UPROPERTY(EditDefaultsOnly,Category="View")
	FVector CameraAimLocalOffset;

	//偏移强度，注意这里不是速度的意义，而是1s内靠近目标的比例
	UPROPERTY(EditDefaultsOnly,Category="View")
	float CameraLerpSpeed=20.f;

	//处理位置逼近的定时器
	FTimerHandle CameraLerpTimerHandle;

	//传入Goal位置并为下一帧设置定时器，调用位置逼近函数
	void LerpCameraToLocalOffset(const FVector& Goal);

	//递归使Camera的位置逼近Goal，实现插值移动
	void TickCameraLocalOffsetLerp(FVector Goal);

	/************************Inventory *********************************/
private:
	UPROPERTY()
	class UInventoryComponent* InventoryComponent;
};
