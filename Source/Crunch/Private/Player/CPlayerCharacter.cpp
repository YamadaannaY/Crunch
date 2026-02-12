// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CPlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Crunch/Crunch.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/CHeroAttributeSet.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Inventory/InventoryComponent.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	CameraBoom=CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	//相机臂跟随Controller(鼠标)一起旋转
	CameraBoom->bUsePawnControlRotation=true;
	
	//只有对SpringArmBlock的对象才会阻挡摄像机
	CameraBoom->ProbeChannel=ECC_SpringArm;
		
	ViewCamera=CreateDefaultSubobject<UCameraComponent>("ViewCamera");
	ViewCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);

	//不让Mesh跟着Controller一起旋转
	bUseControllerRotationYaw=false;
	
	GetCharacterMovement()->bOrientRotationToMovement=true;
	GetCharacterMovement()->RotationRate=FRotator(0,720.f,0.0f);

	HeroAttributesSet=CreateDefaultSubobject<UCHeroAttributeSet>("Hero Attributes Set ");
	InventoryComponent=CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
}

void ACPlayerCharacter::PawnClientRestart()
{
	//这段代码只会在本地客户端生效
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

		//使用Down作为Trigger，具有持续性，有开始和结束两个阶段，分别对应两段逻辑
		EnhancedInputComp->BindAction(LearnAbilityLearnLeaderAction,ETriggerEvent::Started,this,&ThisClass::LearnAbilityLeaderDown);
		EnhancedInputComp->BindAction(LearnAbilityLearnLeaderAction,ETriggerEvent::Completed,this,&ThisClass::LearnAbilityLeaderUp);
		
		for (const TPair<ECAbilityInputID,UInputAction*>& InputActionPair:GameplayAbilityInputAction)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value,ETriggerEvent::Triggered,this,&ThisClass::HandleAbilityInput,InputActionPair.Key);
		}

		EnhancedInputComp->BindAction(UseInventoryITemAction,ETriggerEvent::Triggered,this,&ACPlayerCharacter::UseInventoryItem);
	}
}

void ACPlayerCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation=ViewCamera->GetComponentLocation();
	OutRotation=GetBaseAimRotation();
}

void ACPlayerCharacter::HandleLookInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal=InputActionValue.Get<FVector2d>();

	AddControllerPitchInput(-InputVal.Y);
	AddControllerYawInput(InputVal.X);
}

void ACPlayerCharacter::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	if (GetIsInFocusMode()) return ;
	
	FVector2D InputVal=InputActionValue.Get<FVector2d>();
	//W+D 组合时输入会是 (1,1)，长度 √2,也就是输入值比直线更快，通过归一化获取单位向量避免
	InputVal.Normalize();

	//将摄像机的坐标系作为位移方向参考，添加映射实现位移
	AddMovementInput(GetMoveFwdDir()*InputVal.Y+GetLookRightDir()*InputVal.X);
}

void ACPlayerCharacter::LearnAbilityLeaderDown(const FInputActionValue& InputActionValue)
{
	bIsLearnAbilityLeaderDown=true;
}

void ACPlayerCharacter::LearnAbilityLeaderUp(const FInputActionValue& InputActionValue)
{
	bIsLearnAbilityLeaderDown=false;
}

void ACPlayerCharacter::HandleAbilityInput(const FInputActionValue& InputActionValue, ECAbilityInputID InputID)
{
	const bool bPressed=InputActionValue.Get<bool>();

	//对于拥有等级的Abilities，配合Leader按键进行触发的结果是升级此GA
	if (bPressed && bIsLearnAbilityLeaderDown)
	{
		UpgradeAbilityWithInputID(InputID);
		return ;
	}
	//触发对应ID下的GA
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)InputID);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)InputID);
	}
	
    //InputID为BasicAttack时发送一个PressedTag，用于UpperCut中
	if (InputID==ECAbilityInputID::BasicAttacks)
	{
		//这个Tag只在UpperCut监听Event中被应用，正常Attack除非主动调用此Tag否则不会触发

		FGameplayTag BasicAttackTag=bPressed ? UCAbilitySystemStatics::GetBasicAttackInputPressedTag() : UCAbilitySystemStatics::GetBasicAttackInputReleasedTag();
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this,BasicAttackTag,FGameplayEventData());
		
		//SetUpInput及其回调函数只在客户端执行，所以这里只有客户端的ASC接受了GameplayEvent，需要特别定义Server端的SendGameplayEvent将SendEvent行为发送到服务端
		Server_SendGameplayEventToSelf(BasicAttackTag,FGameplayEventData());
	}
}

void ACPlayerCharacter::UseInventoryItem(const FInputActionValue& InputActionValue)
{
	//在IA设置中定义了Scaler,这里获得Input对应的Int值（1-6）
	int Value=FMath::RoundToInt(InputActionValue.Get<float>());

	//激活对应Slot的Item
	InventoryComponent->TryActivateItemInSlot(Value-1);
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
	//左手定则,以大拇指为A，食指为B，中指即结果Vector
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

void ACPlayerCharacter::OnAimStatChanged(bool bIsAiming)
{
	LerpCameraToLocalOffset(bIsAiming ? CameraAimLocalOffset : FVector{0.f});	
}

void ACPlayerCharacter::LerpCameraToLocalOffset(const FVector& Goal)
{	
	GetWorldTimerManager().ClearTimer(CameraLerpTimerHandle);
	CameraLerpTimerHandle=GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this,&ThisClass::TickCameraLocalOffsetLerp,Goal));
}

void ACPlayerCharacter::TickCameraLocalOffsetLerp(FVector Goal)
{
	//当前摄像机位置
	const FVector CurrentLocalOffset =CameraBoom->SocketOffset;	/* ViewCamera->GetRelativeLocation();*/
	
	//距离小于1uu时递归结束
	if (FVector::Dist(CurrentLocalOffset,Goal) < 1.f)
	{
		/*ViewCamera->SetRelativeLocation(Goal);*/
		
		CameraBoom->SocketOffset = Goal;
		
		//保证Camera在偏移过程严格位于Socket位置
		ViewCamera->SetRelativeLocation(FVector::ZeroVector);

		return ;
	}

	//使用真实帧时间，Alpha即一帧内打算靠近目标多少
	const float LerpAlpha=FMath::Clamp(GetWorld()->GetDeltaSeconds() * CameraLerpSpeed,0.f,1.f);

	//Lerp::  New=Current+(Goal-Current)*Alpha ，插值的意义在于补全移动过程，而Alpha决定了其强度
	const FVector NewLocalOffset=FMath::Lerp(CurrentLocalOffset,Goal,LerpAlpha);

	
	//更新Loc
	/*ViewCamera->SetRelativeLocation(NewLocalOffset);*/

	CameraBoom->SocketOffset = NewLocalOffset;
	ViewCamera->SetRelativeLocation(FVector::ZeroVector);

	//使用一个帧执行一次的一次性定时器进行函数递归调用逼近Goal位置
	CameraLerpTimerHandle=GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this,&ThisClass::TickCameraLocalOffsetLerp,Goal));
}