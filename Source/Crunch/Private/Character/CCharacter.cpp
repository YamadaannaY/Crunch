// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"
#include "net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Crunch/Crunch.h"
#include "Widgets/OverHeadStatsGauge.h"

// Sets default values
ACCharacter::ACCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//Mesh本身没有碰撞，依赖Box进行碰撞判定
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	//碰撞胶囊体忽略SpringArm和Target，避免Character类对象与SpringArm和Target的碰撞
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_SpringArm,ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Target,ECR_Ignore);

	CAbilitySystemComponent=CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	CAttributeSet=CreateDefaultSubobject<UCAttributeSet>("CAttribute Set");

	OverHeadWidgetComponent=CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());

	PerceptionStimuliSourceComponent=CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception StimuliSource Component");

	BindGASChangeDelegates();
}

void ACCharacter::ServerSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this,this);
	
	//在服务端对GA、GE、Attribute进行赋值
	CAbilitySystemComponent->ServerSideInit();
}

void ACCharacter::ClientSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this,this);
}

bool ACCharacter::IsLocallyControlledByPlayer()
{
	//判断LocalController从而找到当前客户端的主Player
	return GetController() && GetController()->IsLocalPlayerController();
}

void ACCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
}

void ACCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCharacter,TeamID);
}

const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& ACCharacter::GetAbilities() const
{
	return CAbilitySystemComponent->GetAbilities();
}

FVector ACCharacter::GetCaptureLocalPosition() const
{
	return HeadShotCaptureLocalPosition;
}

FRotator ACCharacter::GetCaptureLocalRotation() const
{
	return HeadShotCaptureLocalRotation;
}

void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//对于客户端中除自己之外所有Character都渲染一次OverHeadUI
	ConfigureOverHeadStatusWidget();

	//获得当前Mesh相对于CapsuleComponent的变换（所谓Transform，指的是一个坐标系的摆放方式，想象Transform是一张坐标纸，而Vector只是其中一个点，Rotator则表明这张纸相对最开始旋转了多少，
	//Scale表示这张纸相对最开始扩张了多少，最终集合成FTransform）
	MeshRelativeTransform=GetMesh()->GetRelativeTransform();

	//为刺激源组件添加视觉刺激，即能够触发AI的Sense_Sight
	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
}

void ACCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UAbilitySystemComponent* ACCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
}

//将SendEvent也发送给服务端
void ACCharacter::Server_SendGameplayEventToSelf_Implementation(const FGameplayTag EventTag,
	const FGameplayEventData& EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this,EventTag,EventData);
}

bool ACCharacter::Server_SendGameplayEventToSelf_Validate(const FGameplayTag EventTag,
	const FGameplayEventData& EventData)
{
	return true;
}

void ACCharacter::UpgradeAbilityWithInputID(ECAbilityInputID InputID)
{
	//通过触发操作对应的触发ID调用升级逻辑
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->Server_UpgradeAbilityWithID(InputID);
	}
}

void ACCharacter::BindGASChangeDelegates()
{
	if (CAbilitySystemComponent)
	{
		//应用GE附带的GrantedTag被添加到目标ASC的时候是默认Replicate的，所以函数也会在客户端被调用，执行相应的逻辑
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this,&ThisClass::DeadTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatTag()).AddUObject(this,&ThisClass::StunTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetAimStatTag()).AddUObject(this,&ThisClass::AimTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetFocusStatTag()).AddUObject(this,&ThisClass::FocusTagUpdated);

		//根据PA修改Hero移动速度
		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMoveSpeedAttribute()).AddUObject(this,&ThisClass::MoveSpeedUpdated);
	}
}

void ACCharacter::DeadTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
	{
		StartDeathSequence();
	}
	else
	{
		Respawn();
	}
}

void ACCharacter::StunTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead()) return;

	if (NewCount != 0)
	{
		OnStun();
		PlayAnimMontage(StunMontage);
	}
	else
	{
		OnRecoveryFromStun();
		StopAnimMontage(StunMontage);
	}
}

void ACCharacter::AimTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	//瞄准逻辑
	SetIsAiming(NewCount!=0);
}

void ACCharacter::FocusTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	bIsInFocusMode = NewCount > 0;
}

void ACCharacter::SetIsAiming(bool bIsAiming)
{
	//让角色跟随视角进行旋转而不是跟随输入旋转，调用具体逻辑
	bUseControllerRotationYaw=bIsAiming;
	GetCharacterMovement()->bOrientRotationToMovement  =!bIsAiming;
	
	OnAimStatChanged(bIsAiming);
}

void ACCharacter::MoveSpeedUpdated(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed=Data.NewValue;
}

void ACCharacter::OnAimStatChanged(bool bIsAiming)
{
	//override in child class ;
}

void ACCharacter::ConfigureOverHeadStatusWidget()
{
	if (!OverHeadWidgetComponent) return ;

	//本地客户端下Character不需要OverheadUI
	if (IsLocallyControlledByPlayer())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		return;
	}
	
	//非LocalPlayer
	UOverHeadStatsGauge* OverHeadStatsGauge=Cast<UOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	if (OverHeadStatsGauge)
	{
		//属性变化时保持动态更新
		OverHeadStatsGauge->ConfigureWithASC(GetAbilitySystemComponent());
		
		OverHeadWidgetComponent->SetHiddenInGame(false);

		//每次调用重置UpdateTime
		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityUpdateTimerHandle,this,&ACCharacter::UpdateHeadGaugeVisibility,HeadStatGaugeVisibilityUpdateGap,true);
	}
}

void ACCharacter::UpdateHeadGaugeVisibility()
{
	//Starting first with local players and then available remote ones
	APawn* LocalPlayerPawn=UGameplayStatics::GetPlayerPawn(this,0);
	
	if(LocalPlayerPawn)
	{
		//当前Character与本地Pawn的距离差值平方
		float DistSquared=FVector::DistSquared(GetActorLocation(),LocalPlayerPawn->GetActorLocation());

		//决定是否显示UI
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared>HeadStatGaugeVisibilityRangeSquared);
	}
}

void ACCharacter::SetStatusGaugeEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		ConfigureOverHeadStatusWidget();
		OverHeadWidgetComponent->SetVisibility(true);
	}
	else
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		OverHeadWidgetComponent->SetVisibility(false);
	}
}

bool ACCharacter::IsDead() const
{
	return GetAbilitySystemComponent()->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag());
}

void ACCharacter::ReSpawnImmediative()
{
	if (HasAuthority())
	{
		//Removes all active effects that grant any of the tags in Tags
		GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UCAbilitySystemStatics::GetDeadStatTag()));
	}
}

void ACCharacter::DeathMontageFinished()
{
	if (IsDead())
	{
		SetRagDollEnabled(true);
	}
}

void ACCharacter::SetRagDollEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		//将Mesh与Capsule解耦，否则Mesh在被物理施加移动时会被拽回去
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		
		//解除RagDoll后将其重新绑定到RootComponent并且恢复到缓存好的Transform上
		GetMesh()->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

void ACCharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		const float MontageDuration=PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTImerHandle,this,&ThisClass::DeathMontageFinished,MontageDuration+DeathMontageFinishTimeShift);
	}
}

void ACCharacter::StartDeathSequence()
{
	OnDead();

	//即使DeadTag被添加，正在执行的GA也不会结束，需要Cancel所有GA
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->CancelAllAbilities();
	}
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);

	//取消这个限制，使得目标因为LaunchCombo最后一段带有击飞效果的攻击死亡时能够正常被击飞
	/*GetCharacterMovement()->SetMovementMode(MOVE_None);*/
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	SetAIPerceptionStimuliSourceEnabled(false);
}

void ACCharacter::Respawn()
{
	OnRespawn();
	SetAIPerceptionStimuliSourceEnabled(true);
	SetRagDollEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	/* GetCharacterMovement()->SetMovementMode(MOVE_Walking); */
	
	//用于让DeathMontage执行BlendOut
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

	//重新复活在最开始生成的位置
	if (HasAuthority() && GetController())
	{
		//弱指针保证PlayerStart不会悬空
		TWeakObjectPtr<AActor> StartSpot=GetController()->StartSpot;

		if (StartSpot.IsValid())
		{
			SetActorTransform(StartSpot->GetActorTransform());
		}
	}

	//应用FullStatGE
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->ApplyFullStatsEffectSelf();
	}
}

void ACCharacter::OnDead()
{
}

void ACCharacter::OnRespawn()
{
}

void ACCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID=NewTeamID;
}

FGenericTeamId ACCharacter::GetGenericTeamId() const
{
	return TeamID;
}

void ACCharacter::OnRep_TeamID()
{
	//override in AI class
}

void ACCharacter::SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled) const 
{
	if (!PerceptionStimuliSourceComponent) return ;

	//是否将刺激源组件添加到感知系统，由于将感知Pawn设置为false，这里可以进行自主判断
	if (bIsEnabled)
	{
		PerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
	else
	{ 
		PerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}

void ACCharacter::OnStun()
{
	//override in character class 
}

void ACCharacter::OnRecoveryFromStun()
{
	//override in character class 
}
