// 悟空旋转挥棍：激活后播放旋转 Montage，同时在角色 Mesh 的棍骨骼/Socket 上动态生成 CollisionBox
// （棍与角色同属一个 SkeletalMesh，Box 附着到棍的 Socket 后随动画一起旋转扫过周围目标）
// 不使用 AN_SendTargetGroup 扫描，改用 Overlap 收集碰撞对象，命中后应用 FGenericDamageEffectDef：
//   DamageEffect = 伤害 GE；PushVelocity = 径向击退（X=水平速度[自身指向目标]，Z=竖直速度，Y 忽略）
// 伤害检测仅在服务端进行（Server-Authoritative）；同一目标受 PerTargetHitInterval 限频，
// 棍每转一圈重新进入目标触发一次 BeginOverlap，滞留 Box 内的目标由定时器周期性补伤

#pragma once

#include "CoreMinimal.h"
#include "Components/SkinnedMeshComponent.h"
#include "GAS/CGameplayAbilitiesType.h"
#include "GAS/CGameplayAbility.h"
#include "GA_StaffSpin_WuKong.generated.h"

class UBoxComponent;

UCLASS()
class CRUNCH_API UGA_StaffSpin_WuKong : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_StaffSpin_WuKong();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	static FGameplayTag GetStaffSpinAbilityTag() { return FGameplayTag::RequestGameplayTag(TEXT("ability.staffspin.wukong")); }

private:
	// 生成碰撞盒并附着到棍 Socket：
	// 服务端 = 真实判定盒（绑定 Overlap 回调）；客户端 = 仅 bShouldDrawDebug 时生成的可视化盒（无碰撞）
	void SpawnStaffCollisionBox();

	// 销毁碰撞盒、停止补伤定时器、清空命中记录
	void RemoveStaffCollisionBox();

	// Overlap 回调：目标进入棍的碰撞盒
	UFUNCTION()
	void OnStaffBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 定时器：对滞留在盒内的目标周期性补伤（BeginOverlap 只在进入瞬间触发一次）
	void TickRehitOverlappingTargets();

	// 统一伤害入口：阵营/死亡/限频校验后应用 DamageGE + 径向击退
	void TryDamageTarget(AActor* TargetActor);

	// 再次按下技能按键时提前结束 GA（延迟一帧注册，避免激活当帧误触发）
	UFUNCTION()
	void OnToggleInputPressed(float TimeWaited);

	// 旋转挥舞 Montage
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* StaffSpinMontage;

	// 棍所在的骨骼/Socket 名（棍与角色为同一 SkeletalMesh）
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	FName StaffSocketName = TEXT("weapon_r");

	// 碰撞盒半尺寸（默认沿 X 轴拉长以覆盖棍身）
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	FVector StaffBoxExtent = FVector(100.f, 15.f, 15.f);

	// 碰撞盒相对 Socket 的偏移（将盒中心对准棍身中段）
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	FTransform StaffBoxAttachOffset;

	// 伤害 GE + 击退速度（PushVelocity：X=水平径向，Z=竖直，Y 忽略）
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	FGenericDamageEffectDef SpinDamageEffectDef;

	// 同一目标两次受击的最小间隔（秒）；<=0 时整个技能对同一目标只命中一次
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	float PerTargetHitInterval = 0.5f;

	// 运行时生成的碰撞盒（仅服务端存在）
	UPROPERTY()
	TObjectPtr<UBoxComponent> StaffCollisionBox;

	// DedicatedServer 上 Mesh 不渲染，默认 AlwaysTickPose 只推进动画不刷新骨骼，棍 Socket 不会转动
	// 旋转期间临时覆盖为 AlwaysTickPoseAndRefreshBones，EndAbility 时还原为此处缓存的原值
	EVisibilityBasedAnimTickOption CachedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	bool bAnimTickOptionOverridden = false;

	// 每个目标最后一次被命中的时间（世界时间）
	TMap<TWeakObjectPtr<AActor>, float> LastHitTimeMap;

	FTimerHandle RehitTimerHandle;
};
