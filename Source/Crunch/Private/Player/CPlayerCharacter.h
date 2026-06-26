//玩家类，具有本地输入，

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
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

	//在客户端的Pawn发生改变时或者ClientTravel进入关卡后调用，会启用输入，重新初始化AbilityInput
	virtual void PawnClientRestart() override;

	//只在本地客户端执行，处理用户输入（这也是不在服务端执行的原因，服务端不需要知道输入是什么）
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
    //修改Eyes概念，以Camera位置为标准，
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
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

	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputAction* CameraZoomInputAction;

	UPROPERTY(EditDefaultsOnly,Category="Input")
	UInputAction* SprintInputAction;

	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void LearnAbilityLeaderDown(const FInputActionValue& InputActionValue);
	void LearnAbilityLeaderUp(const FInputActionValue& InputActionValue);
	void HandleAbilityInput(const FInputActionValue& InputActionValue,ECAbilityInputID InputID);
	void UseInventoryItem(const FInputActionValue& InputActionValue);
	void HandleCameraZoomInput(const FInputActionValue& InputActionValue);
	void HandleJumpInput();
	void HandleSprintStart(const FInputActionValue& InputActionValue);
	void HandleSprintStop(const FInputActionValue& InputActionValue);

	void SetInputEnabledFromPlayerController(bool bEnabled);

	FVector GetLookRightDir() const ;
	FVector GetLookFwdDir() const ;
	FVector GetMoveFwdDir() const ;

	bool bIsLearnAbilityLeaderDown=false;

	/******************* Movement Smoothing **************************/

	//平滑后的移动输入（当前实际使用的输入值）
	FVector2D SmoothedMoveInput;

	//输入平滑速度，值越大响应越快，越小转向越平滑
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float MoveInputSmoothingSpeed = 8.f;

	//角色转身速率（°/s），值越小转身越平滑
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float TurnRotationRate = 360.f;

	/******************* Double Jump **************************/

	// 最大跳跃次数（映射到 CharacterMovement->JumpMaxCount）
	UPROPERTY(EditDefaultsOnly, Category="Jump")
	int MaxJumpCount = 2;

	// 第一段跳跃的Z轴速度
	UPROPERTY(EditDefaultsOnly, Category="Jump")
	float FirstJumpZVelocity = 600.f;

	// 第二段跳跃的Z轴速度
	UPROPERTY(EditDefaultsOnly, Category="Jump")
	float SecondJumpZVelocity = 450.f;

	// 默认空中控制系数
	UPROPERTY(EditDefaultsOnly, Category="Jump")
	float DefaultAirControl = 0.2f;

	// 二段跳时的空中控制系数（值越大空中转向越灵活）
	UPROPERTY(EditDefaultsOnly, Category="Jump")
	float DoubleJumpAirControl = 0.5f;

	// 落地时重置跳跃和空中控制状态
	virtual void Landed(const FHitResult& Hit) override;

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

	//偏移强度，注意这里不是速度的概念，而是1s内靠近目标的比例
	UPROPERTY(EditDefaultsOnly,Category="View")
	float CameraLerpSpeed=20.f;

	//处理位置逼近的定时器
	FTimerHandle CameraLerpTimerHandle;

	//传入Goal位置并为下一帧设置定时器，调用位置逼近函数
	void LerpCameraToLocalOffset(const FVector& Goal);

	//递归使Camera的位置逼近Goal，实现插值移动
	void TickCameraLocalOffsetLerp(FVector Goal);

	/******************* Camera Zoom **************************/

	//弹簧臂最小长度
	UPROPERTY(EditDefaultsOnly,Category="View|Zoom")
	float MinArmLength=200.f;

	//弹簧臂最大长度
	UPROPERTY(EditDefaultsOnly,Category="View|Zoom")
	float MaxArmLength=800.f;

	//鼠标滚轮每格的缩放步长
	UPROPERTY(EditDefaultsOnly,Category="View|Zoom")
	float ZoomStepSize=50.f;

	//缩放的Lerp速度
	UPROPERTY(EditDefaultsOnly,Category="View|Zoom")
	float ZoomLerpSpeed=10.f;

	//是否处于瞄准状态
	bool bIsAim;
	
	//目标臂长
	float TargetArmLength;

	FTimerHandle ArmLengthLerpTimerHandle;

	//维护一个定时器，作为递归的入口函数
	void LerpArmLength(float Goal);
	//递归函数
	void TickArmLengthLerp(float Goal);

	/************************ Sprint *********************************/
private:
	// 冲刺时的 MaxWalkSpeed（替代默认 MoveSpeed 属性值）
	UPROPERTY(EditDefaultsOnly, Category="Sprint")
	float SprintSpeed = 900.f;

	// 冲刺 GE：服务端应用，修改 MoveSpeed 属性（Duration=Infinite）
	UPROPERTY(EditDefaultsOnly, Category="Sprint")
	TSubclassOf<UGameplayEffect> SprintEffect;

	// 服务端冲刺 GE 的 ActiveHandle，用于停止时移除
	FActiveGameplayEffectHandle SprintEffectHandle;

	// RPC：服务端开始/停止冲刺
	UFUNCTION(Server, Reliable)
	void Server_StartSprint();
	UFUNCTION(Server, Reliable)
	void Server_StopSprint();

	/************************Inventory *********************************/
private:
	UPROPERTY()
	class UInventoryComponent* InventoryComponent;
};
