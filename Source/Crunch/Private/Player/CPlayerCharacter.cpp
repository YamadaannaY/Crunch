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
	//相机臂跟随Controller(AddInput的变化)一起旋转
	//只有对SpringArmBlock的对象才会阻挡摄像机
	CameraBoom=CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation=true;
	CameraBoom->ProbeChannel=ECC_SpringArm;

	ViewCamera=CreateDefaultSubobject<UCameraComponent>("ViewCamera");
	ViewCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);

	//不让Mesh跟着Controller一起旋转
	bUseControllerRotationYaw=false;
	
	GetCharacterMovement()->bOrientRotationToMovement=true;
	GetCharacterMovement()->RotationRate=FRotator(0,TurnRotationRate,0.0f);

	HeroAttributesSet=CreateDefaultSubobject<UCHeroAttributeSet>("Hero Attributes Set ");
	InventoryComponent=CreateDefaultSubobject<UInventoryComponent>("Inventory Component");

	TargetArmLength=CameraBoom->TargetArmLength;

	// 二段跳初始化
	JumpMaxCount=MaxJumpCount;
	GetCharacterMovement()->JumpZVelocity=FirstJumpZVelocity;
	GetCharacterMovement()->AirControl=DefaultAirControl;
}

void ACPlayerCharacter::PawnClientRestart()
{
	//只在本地客户端生效
	
	Super::PawnClientRestart();
	
	if (APlayerController* OwningPlayerController=GetController<APlayerController>())
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
	
	if (UEnhancedInputComponent* EnhancedInputComp=Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Trigger作为触发，具有瞬时性,不处理Completed
		EnhancedInputComp->BindAction(JumpInputAction,ETriggerEvent::Triggered,this,&ThisClass::HandleJumpInput);
		EnhancedInputComp->BindAction(LookInputAction,ETriggerEvent::Triggered,this,&ThisClass::HandleLookInput);
		EnhancedInputComp->BindAction(MoveInputAction,ETriggerEvent::Triggered,this,&ThisClass::HandleMoveInput);
		EnhancedInputComp->BindAction(UseInventoryITemAction,ETriggerEvent::Triggered,this,&ACPlayerCharacter::UseInventoryItem);
		EnhancedInputComp->BindAction(CameraZoomInputAction,ETriggerEvent::Triggered,this,&ThisClass::HandleCameraZoomInput);

		//使用Down作为触发，具有持续性，有开始和结束两个回调
		EnhancedInputComp->BindAction(LearnAbilityLearnLeaderAction,ETriggerEvent::Triggered,this,&ThisClass::LearnAbilityLeaderDown);
		EnhancedInputComp->BindAction(LearnAbilityLearnLeaderAction,ETriggerEvent::Completed,this,&ThisClass::LearnAbilityLeaderUp);

		// Sprint：按住加速，松手恢复
		EnhancedInputComp->BindAction(SprintInputAction,ETriggerEvent::Started,this,&ThisClass::HandleSprintStart);
		EnhancedInputComp->BindAction(SprintInputAction,ETriggerEvent::Completed,this,&ThisClass::HandleSprintStop);

		for (const TPair<ECAbilityInputID,UInputAction*>& InputActionPair:GameplayAbilityInputAction)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value,ETriggerEvent::Triggered,
				this,&ThisClass::HandleAbilityInput,InputActionPair.Key);
		}
	}
}

void ACPlayerCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation=ViewCamera->GetComponentLocation();
	OutRotation=GetBaseAimRotation();
}

void ACPlayerCharacter::HandleLookInput(const FInputActionValue& InputActionValue)
{
	const FVector2D InputVal=InputActionValue.Get<FVector2d>();

	AddControllerPitchInput(-InputVal.Y);
	AddControllerYawInput(InputVal.X);
}

void ACPlayerCharacter::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	if (GetIsInFocusMode()) return ;
	
	//W+D 组合输入时会是 (1,1)，向量长度 √2,也就是输入值比直线更快，通过归一化获取单位向量进行避免
	FVector2D InputVal=InputActionValue.Get<FVector2d>();
	InputVal.Normalize();

	// 对输入做平滑插值添加，避免方向瞬间跳变导致角色旋转和动画生硬
	float DeltaTime=GetWorld()->GetDeltaSeconds();
	SmoothedMoveInput=FMath::Vector2DInterpTo(SmoothedMoveInput,InputVal,DeltaTime,MoveInputSmoothingSpeed);
	AddMovementInput(GetMoveFwdDir()*SmoothedMoveInput.Y+GetLookRightDir()*SmoothedMoveInput.X);
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

	//对于拥有等级的Abilities，配合Leader按键进行技能触发的操作是升级此GA
	if (bPressed && bIsLearnAbilityLeaderDown)
	{
		UpgradeAbilityWithInputID(InputID);
		return ;
	}
	
	if (bPressed)
	{
		// 二段跳期间禁止触发普攻 GA（一段跳可以）
		if (InputID == ECAbilityInputID::BasicAttacks && JumpCurrentCount >= 2)
		{
			return;
		}
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
		const FGameplayTag BasicAttackTag=bPressed ? UCAbilitySystemStatics::GetBasicAttackInputPressedTag() : UCAbilitySystemStatics::GetBasicAttackInputReleasedTag();
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this,BasicAttackTag,FGameplayEventData());

		//将Input触发的Event发送给服务端
		Server_SendGameplayEventToSelf(BasicAttackTag,FGameplayEventData());
	}
}

void ACPlayerCharacter::UseInventoryItem(const FInputActionValue& InputActionValue)
{
	//在IA设置中定义了Scaler,可以直接获得Input对应的Int值（1-6）
	const int Value=FMath::RoundToInt(InputActionValue.Get<float>());

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
	// 死亡时强制结束冲刺
	if (HasAuthority())
	{
		Server_StopSprint_Implementation();
	}
	SetInputEnabledFromPlayerController(false);
}

void ACPlayerCharacter::OnRespawn()
{
	SetInputEnabledFromPlayerController(true);
}

void ACPlayerCharacter::OnStun()
{
	// 眩晕时强制结束冲刺
	if (HasAuthority())
	{
		Server_StopSprint_Implementation();
	}
	SetInputEnabledFromPlayerController(false);
}

void ACPlayerCharacter::OnRecoveryFromStun()
{
	if (IsDead()) return;
	SetInputEnabledFromPlayerController(true);
}

void ACPlayerCharacter::OnAimStatChanged(bool bIsAiming)
{
	bIsAim=bIsAiming;
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

	//Lerp::  New=Current+(Goal-Current)*Alpha ，插值补全移动过程
	const FVector NewLocalOffset=FMath::Lerp(CurrentLocalOffset,Goal,LerpAlpha);

	
	//更新Loc
	/*ViewCamera->(NewLocalOffset);*/

	CameraBoom->SocketOffset = NewLocalOffset;
	ViewCamera->SetRelativeLocation(FVector::ZeroVector);

	//使用一个帧执行一次的一次性定时器进行函数递归调用逼近Goal位置
	CameraLerpTimerHandle=GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this,&ThisClass::TickCameraLocalOffsetLerp,Goal));
}

void ACPlayerCharacter::HandleCameraZoomInput(const FInputActionValue& InputActionValue)
{
	if (bIsAim) return ;
	
	const float ZoomValue=InputActionValue.Get<float>();
	TargetArmLength=FMath::Clamp(TargetArmLength + ZoomValue * ZoomStepSize, MinArmLength, MaxArmLength);

	LerpArmLength(TargetArmLength);
}

void ACPlayerCharacter::LerpArmLength(float Goal)
{
	GetWorldTimerManager().ClearTimer(ArmLengthLerpTimerHandle);
	ArmLengthLerpTimerHandle=GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this,&ThisClass::TickArmLengthLerp,Goal));
}

void ACPlayerCharacter::TickArmLengthLerp(float Goal)
{
	const float CurrentArmLength=CameraBoom->TargetArmLength;

	//值太小不用递归
	if (FMath::Abs(CurrentArmLength - Goal) < 1.f)
	{
		CameraBoom->TargetArmLength=Goal;
		return;
	}

	//一帧内移动的速度
	const float LerpAlpha=FMath::Clamp(GetWorld()->GetDeltaSeconds() * ZoomLerpSpeed, 0.f, 1.f);
	const float NewArmLength=FMath::Lerp(CurrentArmLength, Goal, LerpAlpha);

	CameraBoom->TargetArmLength=NewArmLength;

	//递归
	ArmLengthLerpTimerHandle=GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this,&ThisClass::TickArmLengthLerp,Goal));
}

void ACPlayerCharacter::HandleJumpInput()
{
	UCharacterMovementComponent* MovementComp=GetCharacterMovement();
	if (!MovementComp) return;
	if (!CanJump()) return;

	if (JumpCurrentCount == 0)
	{
		// 地面第一段跳
		MovementComp->JumpZVelocity=FirstJumpZVelocity;
	}
	else
	{
		// 空中二段跳：使用较低的跳跃速度和更高的空中控制
		MovementComp->JumpZVelocity=SecondJumpZVelocity;
		MovementComp->AirControl=DoubleJumpAirControl;

		// 二段跳时取消普攻、Roll、StaffSpin 技能
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(FGameplayTag::RequestGameplayTag("ability.staffspin.wukong"));
		CancelTags.AddTag(FGameplayTag::RequestGameplayTag("ability.basicattack"));
		GetAbilitySystemComponent()->CancelAbilities(&CancelTags); // 本地取消（LocalPredicted GA）
		Server_CancelDoubleJumpAbilities();                        // 服务端取消（ServerOnly GA）
	}

	Jump();
}

void ACPlayerCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	// 服务端广播跳跃事件到所有客户端，确保远程客户端每次都收到正确的跳跃段数
	// 解决 JumpCurrentCount 复制跳过中间值（0→2）导致远程客户端动画错乱
	if (HasAuthority())
	{
		Multicast_PlayJumpAnimation(JumpCurrentCount);
	}
}

void ACPlayerCharacter::Multicast_PlayJumpAnimation_Implementation(int32 InJumpCount)
{
	// 直接修正远程客户端的 JumpCurrentCount，触发 OnRep → OnJumped → ABP 读取正确的 GetJumpCount()
	// 本地客户端 JumpCurrentCount 已由本地预测正确设置，相同值不会触发 OnRep
	JumpCurrentCount = InJumpCount;
}

void ACPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// 落地时恢复默认空中控制系数
	GetCharacterMovement()->AirControl=DefaultAirControl;
}

/**************************** Sprint ******************************/

void ACPlayerCharacter::HandleSprintStart(const FInputActionValue& InputActionValue)
{
	Server_StartSprint();
}

void ACPlayerCharacter::HandleSprintStop(const FInputActionValue& InputActionValue)
{
	Server_StopSprint();
}

void ACPlayerCharacter::Server_StartSprint_Implementation()
{
	if (!SprintEffect)
	{
		// 没有配置 GE 时直接修改 MaxWalkSpeed
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	// 避免重复应用
	if (SprintEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(SprintEffectHandle);
	}

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(SprintEffect, 1, ASC->MakeEffectContext());
	SprintEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void ACPlayerCharacter::Server_StopSprint_Implementation()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	if (SprintEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(SprintEffectHandle);
		SprintEffectHandle = FActiveGameplayEffectHandle();
	}
}

void ACPlayerCharacter::Server_CancelDoubleJumpAbilities_Implementation()
{
	FGameplayTagContainer CancelTags;
	CancelTags.AddTag(FGameplayTag::RequestGameplayTag("ability.staffspin.wukong"));
	CancelTags.AddTag(FGameplayTag::RequestGameplayTag("ability.basicattack"));
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->CancelAbilities(&CancelTags);
	}
}
