// SandboxCharacter 实现 —— Walk/Run 切换 + SandboxAnimInstance 驱动

#include "Sandbox/SandboxCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ASandboxCharacter::ASandboxCharacter()
{
}

void ASandboxCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 如果 RunSpeed 为 0，使用当前属性驱动的 MaxWalkSpeed 作为 Run 速度
	if (RunSpeed <= 0.f)
	{
		RunSpeed = GetCharacterMovement()->MaxWalkSpeed;
	}
}

void ASandboxCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Ctrl 点按：切换 Walk / Run（只绑定 Started，不绑定 Completed）
		if (CtrlInputAction)
		{
			EnhancedInputComp->BindAction(CtrlInputAction, ETriggerEvent::Started, this, &ThisClass::HandleCtrlToggle);
		}
	}
}

void ASandboxCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASandboxCharacter, bWalkMode);
}

// ──── Walk / Run Toggle ───────────────────────────────────────

void ASandboxCharacter::HandleCtrlToggle(const FInputActionValue& InputActionValue)
{
	Server_ToggleWalkMode();
}

void ASandboxCharacter::Server_ToggleWalkMode_Implementation()
{
	bWalkMode = !bWalkMode;

	const float TargetSpeed = bWalkMode ? WalkSpeed : RunSpeed;
	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}
