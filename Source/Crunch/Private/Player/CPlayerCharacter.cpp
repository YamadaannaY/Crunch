// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	CameraBoom=CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	//相机臂跟随Controller(鼠标)一起旋转
	CameraBoom->bUsePawnControlRotation=true;
	
	ViewCamera=CreateDefaultSubobject<UCameraComponent>("ViewCamera");
	ViewCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);

	//不让Mesh跟着Controller一起旋转
	bUseControllerRotationYaw=false;
	
	GetCharacterMovement()->bOrientRotationToMovement=true;
	GetCharacterMovement()->RotationRate=FRotator(0,720.f,0.0f);
}

void ACPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	APlayerController* OwningPlayerController=GetController<APlayerController>();
	if (OwningPlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem=OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

		if (InputSubsystem)
		{
			InputSubsystem->RemoveMappingContext(GameplayInputMappingContext);
			InputSubsystem->AddMappingContext(GameplayInputMappingContext,0);
		}
	}
}

void ACPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComp=Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComp)
	{
		//Jump是内置的基础跳跃函数
		EnhancedInputComp->BindAction(JumpInputAction,ETriggerEvent::Triggered,this,&ThisClass::Jump);
		EnhancedInputComp->BindAction(LookInputAction,ETriggerEvent::Triggered,this,&ThisClass::HandleLookInput);
		EnhancedInputComp->BindAction(MoveInputAction,ETriggerEvent::Triggered,this,&ThisClass::HandleMoveInput);

		for (const TPair<ECAbilityInputID,UInputAction*>& InputActionPair:GameplayAbilityInputAction)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value,ETriggerEvent::Triggered,this,&ThisClass::HandleAbilityInput,InputActionPair.Key);
		}
	}
}

void ACPlayerCharacter::HandleLookInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal=InputActionValue.Get<FVector2d>();

	AddControllerPitchInput(-InputVal.Y);
	AddControllerYawInput(InputVal.X);
}

void ACPlayerCharacter::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal=InputActionValue.Get<FVector2d>();
	//W+D 组合时输入会是 (1,1)，长度 √2,也就是输入值比直线更快，通过归一化避免
	InputVal.Normalize();

	//将摄像机的坐标系作为位移方向参考，添加映射实现位移
	AddMovementInput(GetMoveFwdDir()*InputVal.Y+GetLookRightDir()*InputVal.X);
}

void ACPlayerCharacter::HandleAbilityInput(const FInputActionValue& InputActionValue, ECAbilityInputID InputID)
{
	bool  bPressed=InputActionValue.Get<bool>();

	//触发对应ID下的GA
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)InputID);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)InputID);
	}
}

FVector ACPlayerCharacter::GetLookRightDir() const
{
	return ViewCamera->GetRightVector();
}

FVector ACPlayerCharacter::GetLookFwdDir() const
{
	return ViewCamera->GetForwardVector();
}

FVector ACPlayerCharacter::GetMoveFwdDir() const
{
	//Right × Up = Forward 向量叉乘，左手定则
	return FVector::CrossProduct(GetLookRightDir(),FVector::UpVector);
}

void ACPlayerCharacter::SetInputEnabledFromPlayerController(bool bEnabled)
{
	APlayerController* PlayerController=GetController<APlayerController>();
	if (!PlayerController)
	{
		return;
	}
	if (bEnabled)
	{
		EnableInput(PlayerController);
	}
	else
	{
		DisableInput(PlayerController);
	}
}

void ACPlayerCharacter::OnDead()
{
	SetInputEnabledFromPlayerController(false);
}

void ACPlayerCharacter::OnRespawn()
{
	SetInputEnabledFromPlayerController(true);
}

void ACPlayerCharacter::OnStun()
{
	SetInputEnabledFromPlayerController(false);
}

void ACPlayerCharacter::OnRecoveryFromStun()
{
	if (IsDead()) return;
	SetInputEnabledFromPlayerController(true);
}
