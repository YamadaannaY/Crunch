#include "GAS/GA_Shoot.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/ProjectileActor.h"
#include "GameplayTagsManager.h"
#include "Kismet/GameplayStatics.h"


UGA_Shoot::UGA_Shoot() : ShootMontage(nullptr),AimTarget(nullptr),AimTargetAbilitySystemComponent(nullptr),bInputLocked(false)
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetCrosshairTag());
}

void UGA_Shoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return ;
	}
	
	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		UAbilityTask_WaitGameplayEvent* WaitStartShootingTask=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UCAbilitySystemStatics::GetBasicAttackInputPressedTag());
		WaitStartShootingTask->EventReceived.AddDynamic(this,&ThisClass::StartShooting);
		WaitStartShootingTask->ReadyForActivation();
		
		UAbilityTask_WaitGameplayEvent* WaitStopShootingTask=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UCAbilitySystemStatics::GetBasicAttackInputReleasedTag());
		WaitStopShootingTask->EventReceived.AddDynamic(this,&ThisClass::StopShooting);
		WaitStopShootingTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootProjectileEvent=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,GetShootTag(),nullptr,false,false);
		WaitShootProjectileEvent->EventReceived.AddDynamic(this,&ThisClass::ShootProjectile);
		WaitShootProjectileEvent->ReadyForActivation();
	}
}

void UGA_Shoot::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	bInputLocked = false;
	
	if (AimTargetAbilitySystemComponent)
	{
		AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).RemoveAll(this);
		AimTargetAbilitySystemComponent=nullptr;
	}

	SendLocalGameplayEvent(UCAbilitySystemStatics::GetTargetUpdatedTag(),FGameplayEventData());

	StopShooting(FGameplayEventData());

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Shoot::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo)
{
	K2_EndAbility();
}

FGameplayTag UGA_Shoot::GetShootTag()
{
	return FGameplayTag::RequestGameplayTag("ability.shoot");
}


void UGA_Shoot::StartAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(AimTargetCheckTimerHandle,this,&ThisClass::FindAimTarget,AimTargetCheckTimeInterval,true);
	}
}

void UGA_Shoot::StopAimTargetCheckTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AimTargetCheckTimerHandle);
	}
}

bool UGA_Shoot::HasValidTarget() const 
{
	if (!AimTarget) return false;
	if (UCAbilitySystemStatics::IsActorDead(AimTarget)) return false;
	if (!IsTargetInRange()) return false;

	return true;
}

void UGA_Shoot::StartShooting(FGameplayEventData PayLoad)
{
	if (bInputLocked) return; // 锁定期间禁止开始

	if (UAnimInstance* AnimInst = GetOwnerAnimInstance())
	{
		if (ShootMontage && AnimInst->Montage_IsPlaying(ShootMontage))
		{
			return;
		}
	}
	
	UAbilityTask_PlayMontageAndWait* PlayShootMontage=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,ShootMontage);
	PlayShootMontage->ReadyForActivation();

	FindAimTarget();
	StartAimTargetCheckTimer();
}

	
void UGA_Shoot::StopShooting(FGameplayEventData PayLoad)
{
	if (bInputLocked) return; // 如果已经在停止，忽略重复调用

	bInputLocked = true; // 锁定，禁止开始

	// 停止瞄准相关逻辑
	StopAimTargetCheckTimer();

	if (UAnimInstance* AnimInst = GetOwnerAnimInstance())
	{
		if (ShootMontage && AnimInst->Montage_IsPlaying(ShootMontage))
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UGA_Shoot::OnShootMontageEnded);
			AnimInst->Montage_SetEndDelegate(EndDelegate, ShootMontage);
			// 设置下一节为 None，当前 section 结束后自然停止
			FName CurrentSection = AnimInst->Montage_GetCurrentSection(ShootMontage);
			AnimInst->Montage_SetNextSection(CurrentSection, NAME_None, ShootMontage);
			return; // 等待异步回调，不立即执行后续解锁
		}
	}

	// 如果没有正在播放的蒙太奇，直接解锁
	bInputLocked = false;
}


void UGA_Shoot::OnShootMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 蒙太奇已停止（无论是自然结束还是被打断），解锁输入
	bInputLocked = false;
}

void UGA_Shoot::ShootProjectile(FGameplayEventData PayLoad)
{
	if (K2_HasAuthority())
	{
		AActor* OwnerAvatarActor=GetAvatarActorFromActorInfo();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner=OwnerAvatarActor;
		SpawnParams.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector SocketLocation=OwnerAvatarActor->GetActorLocation();
		USkeletalMeshComponent* MeshComp=GetOwningComponentFromActorInfo();
		if (MeshComp)
		{
			TArray<FName> OutNames;
			UGameplayTagsManager::Get().SplitGameplayTagFName(PayLoad.EventTag,OutNames);
			if (OutNames.Num()>0)
			{
				FName SocketName=OutNames.Last();
				SocketLocation=MeshComp->GetSocketLocation(SocketName);
			}
		}

		AProjectileActor* Projectile=GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass,SocketLocation,OwnerAvatarActor->GetActorRotation(),SpawnParams);
		if (Projectile)
		{
			Projectile->ShootProjectile(ShootProjectileSpeed,ShootProjectileRange,GetAimTargetIfValid(),GetOwnerTeamId(),
			MakeOutgoingGameplayEffectSpec(ProjectileEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo)));
		}
	}
}

AActor* UGA_Shoot::GetAimTargetIfValid() const
{
	if (HasValidTarget())
	{
		return AimTarget;
	}
	return nullptr;
}

void UGA_Shoot::FindAimTarget()
{
	if (HasValidTarget())
	{
		AController* Controller = Cast<APawn>(GetAvatarActorFromActorInfo())->GetController();
		if (Controller)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Controller);
			if (IsActorInCameraFrustum(AimTarget,PlayerController))
			{
				return;
			}
		}	
	}
	if (AimTargetAbilitySystemComponent)
	{
		//每次调用都重新寻找AimTarget，需要重置ASC
		AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).RemoveAll(this);
		AimTargetAbilitySystemComponent=nullptr;
	}
	
	AimTarget=GetAimTarget(ShootProjectileRange,ETeamAttitude::Hostile);
	if (AimTarget)
	{
		AimTargetAbilitySystemComponent=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AimTarget);
		if (AimTargetAbilitySystemComponent)
		{
			AimTargetAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this,&ThisClass::TargetDeadTagUpdated);
		}
	}

	FGameplayEventData EventData;
	EventData.Target=AimTarget;
	SendLocalGameplayEvent(UCAbilitySystemStatics::GetTargetUpdatedTag(),EventData);
}

void UGA_Shoot::TargetDeadTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		FindAimTarget();
	}
}

bool UGA_Shoot::IsTargetInRange() const
{
	if (!AimTarget) return false;

	const float Dist=FVector::Distance(AimTarget->GetActorLocation(),GetAvatarActorFromActorInfo()->GetActorLocation());

	return Dist<=ShootProjectileRange;
}

bool UGA_Shoot::IsActorInCameraFrustum(AActor* Actor, APlayerController* PlayerController)
{
	if (!Actor || !PlayerController || !PlayerController->PlayerCameraManager)
		return false;

	// 获取摄像机视图
	FMinimalViewInfo ViewInfo = PlayerController->PlayerCameraManager->GetCameraCacheView();
    
	// 获取矩阵
	FMatrix ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;
	UGameplayStatics::GetViewProjectionMatrix(ViewInfo, ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);
    
	// 构建视锥体
	FConvexVolume Frustum;
	GetViewFrustumBounds(Frustum, ViewProjectionMatrix, true, true);
    
	// 获取Actor包围盒
	FBoxSphereBounds Bounds = Actor->GetRootComponent()->Bounds;
    
	// 检测是否在视锥体内
	return Frustum.IntersectBox(Bounds.Origin, Bounds.BoxExtent);
}