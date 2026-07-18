// 悟空旋转挥棍：CollisionBox 附着到棍 Socket 随动画旋转，Overlap 收集目标并应用 FGenericDamageEffectDef
// 命中管线：BeginOverlap（棍每转一圈重新进入目标触发一次）+ 定时器补伤（目标滞留盒内时）
//   → TryDamageTarget 统一校验（阵营/死亡/限频）→ 径向击退 + DamageGE
// DedicatedServer 适配：服务端 Mesh 不渲染，默认 AlwaysTickPose 不刷新骨骼、Socket 不随棍转动，
// 因此旋转期间临时把 Mesh 的 VisibilityBasedAnimTickOption 覆盖为 AlwaysTickPoseAndRefreshBones，结束时还原

#include "GA_StaffSpin_WuKong.h"
#include "GAS/UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Crunch/DebugHelper.h"

UGA_StaffSpin_WuKong::UGA_StaffSpin_WuKong()
{
	SetAssetTags(FGameplayTagContainer(GetStaffSpinAbilityTag()));
	BlockAbilitiesWithTag.AddTag(GetStaffSpinAbilityTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_StaffSpin_WuKong::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility() || !StaffSpinMontage)
	{
		K2_EndAbility();
		return;
	}
	
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlaySpinMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, StaffSpinMontage);
		PlaySpinMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::K2_EndAbility);
		PlaySpinMontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
		PlaySpinMontageTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
		PlaySpinMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
		PlaySpinMontageTask->ReadyForActivation();

		// 延迟一帧注册 WaitInputPress，避免激活当帧的 InputPressed 事件立即误触发结束
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (IsActive())
			{
				UAbilityTask_WaitInputPress* WaitPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
				WaitPress->OnPress.AddDynamic(this, &ThisClass::OnToggleInputPressed);
				WaitPress->ReadyForActivation();
			}
		});
	}

	// 伤害检测仅在服务端：所以服务端生成碰撞盒 + 启动补伤定时器(可选时间)
	if (K2_HasAuthority())
	{
		LastHitTimeMap.Empty();
		SpawnStaffCollisionBox();

		if (StaffCollisionBox && PerTargetHitInterval > 0.f)
		{
			GetWorld()->GetTimerManager().SetTimer(RehitTimerHandle, this, &ThisClass::TickRehitOverlappingTargets, PerTargetHitInterval, true);
		}
	}
	//调试用：在本地客户端额外生成一个纯可视化 Box（无碰撞、不参与伤害），观察位置/尺寸/随棍转动
	else if (ShouldDrawDebug())
	{
		SpawnStaffCollisionBox();
	}
}

void UGA_StaffSpin_WuKong::SpawnStaffCollisionBox()
{
	ACharacter* OwnerCharacter = GetOwningAvatarCharacter();
	if (!OwnerCharacter || !OwnerCharacter->GetMesh()) return;

	USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh();
	const bool bIsServer = K2_HasAuthority();

	// 运行时动态生成 Box，附着到棍的 Socket（棍与角色同属一个 SkeletalMesh），随挥舞动画一起转动
	StaffCollisionBox = NewObject<UBoxComponent>(OwnerCharacter);
	StaffCollisionBox->RegisterComponent();
	StaffCollisionBox->AttachToComponent(OwnerMesh, FAttachmentTransformRules::KeepRelativeTransform, StaffSocketName);
	StaffCollisionBox->SetRelativeTransform(StaffBoxAttachOffset);
	StaffCollisionBox->SetBoxExtent(StaffBoxExtent);

	if (bIsServer)
	{
		// DS 上 Mesh 不渲染时骨骼默认不刷新（AlwaysTickPose 只推进动画），
		// 旋转期间临时强制刷新骨骼，让棍 Socket（及附着的 Box）随挥舞动画转动；EndAbility 还原
		CachedAnimTickOption = OwnerMesh->VisibilityBasedAnimTickOption;
		OwnerMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		bAnimTickOptionOverridden = true;

		// 仅做 Query Overlap，且只与 Pawn 产生 Overlap
		StaffCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		StaffCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
		StaffCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		StaffCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		StaffCollisionBox->SetGenerateOverlapEvents(true);
		StaffCollisionBox->IgnoreActorWhenMoving(OwnerCharacter, true);

		StaffCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnStaffBoxBeginOverlap);
	}
	else
	{
		// 客户端调试用可视化 Box：完全关闭碰撞，只显示线框
		StaffCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Debug：显示碰撞盒线框（ShapeComponent 默认 HiddenInGame）
	if (ShouldDrawDebug())
	{
		StaffCollisionBox->SetHiddenInGame(false);
	}
}

void UGA_StaffSpin_WuKong::OnStaffBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryDamageTarget(OtherActor);
}

void UGA_StaffSpin_WuKong::TickRehitOverlappingTargets()
{
	if (!StaffCollisionBox || !IsActive()) return;

	// 对仍滞留在盒内的目标按间隔重复命中（BeginOverlap 只在进入瞬间触发）
	TArray<AActor*> OverlappingActors;
	StaffCollisionBox->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
	for (AActor* OverlappingActor : OverlappingActors)
	{
		TryDamageTarget(OverlappingActor);
	}
}

void UGA_StaffSpin_WuKong::TryDamageTarget(AActor* TargetActor)
{
	AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
	if (!TargetActor || !OwnerAvatarActor || TargetActor == OwnerAvatarActor) return;

	// 只命中敌对目标；跳过已死亡目标（对象池中的 Minion 尸体仍留在场景内）
	if (!IsActorTeamAttitudeIs(TargetActor, ETeamAttitude::Hostile)) return;
	if (UCAbilitySystemStatics::IsActorDead(TargetActor)) return;

	// 同一目标限频：两次命中间隔需大于 PerTargetHitInterval；<=0 时只允许命中一次
	const float Now = GetWorld()->GetTimeSeconds();
	if (const float* LastHitTime = LastHitTimeMap.Find(TargetActor))
	{
		if (PerTargetHitInterval <= 0.f) return;
		if (Now - *LastHitTime < PerTargetHitInterval) return;
	}
	LastHitTimeMap.Add(TargetActor, Now);

	// 径向击退：旋转挥舞对四周目标的击退方向为 自身→目标（水平），PushVelocity.X=水平速度，Z=竖直速度
	const FVector RadialDir = (TargetActor->GetActorLocation() - OwnerAvatarActor->GetActorLocation()).GetSafeNormal2D();
	const FVector PushVel = RadialDir * SpinDamageEffectDef.PushVelocity.X + FVector::UpVector * SpinDamageEffectDef.PushVelocity.Z;
	if (!PushVel.IsNearlyZero())
	{
		PushTarget(TargetActor, PushVel);
	}

	// 构造 HitResult 记录进 GE Context（供 GameplayCue 取命中位置/朝向）
	if (SpinDamageEffectDef.DamageEffect)
	{
		FHitResult HitResult(TargetActor, Cast<UPrimitiveComponent>(TargetActor->GetRootComponent()), TargetActor->GetActorLocation(), -RadialDir);
		ApplyGameplayEffectToHitResultActor(HitResult, SpinDamageEffectDef.DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
	}

	if (ShouldDrawDebug())
	{
		Debug::Print(FString::Printf(TEXT("[WuKong StaffSpin] Hit %s"), *TargetActor->GetName()), FColor::Cyan);
	}
}

void UGA_StaffSpin_WuKong::RemoveStaffCollisionBox()
{
	if (RehitTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(RehitTimerHandle);
	}

	if (StaffCollisionBox)
	{
		StaffCollisionBox->OnComponentBeginOverlap.RemoveAll(this);
		StaffCollisionBox->DestroyComponent();
		StaffCollisionBox = nullptr;
	}

	// 还原 Mesh 的动画 Tick 选项（DS 骨骼刷新只在旋转期间开启）
	if (bAnimTickOptionOverridden)
	{
		if (const ACharacter* OwnerCharacter = GetOwningAvatarCharacter())
		{
			if (USkeletalMeshComponent* OwnerMesh = OwnerCharacter->GetMesh())
			{
				OwnerMesh->VisibilityBasedAnimTickOption = CachedAnimTickOption;
			}
		}
		bAnimTickOptionOverridden = false;
	}

	LastHitTimeMap.Empty();
}

void UGA_StaffSpin_WuKong::OnToggleInputPressed(float TimeWaited)
{
	K2_EndAbility();
}

void UGA_StaffSpin_WuKong::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	RemoveStaffCollisionBox();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}