
#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GA_Shoot.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Shoot : public UCGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Shoot();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	//在LocalInputReleased（ID）触发的时候调用，即松开按键时
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
	UPROPERTY(EditDefaultsOnly,Category="Anim")
	UAnimMontage* ShootMontage;

	UPROPERTY(EditDefaultsOnly,Category="Shoot")
	float ShootProjectileSpeed=2000.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Shoot")
	float ShootProjectileRange=3000.f;
	
	UPROPERTY(EditDefaultsOnly,Category="Shoot")
	TSubclassOf<class AProjectileActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly,Category="Shoot")
	TSubclassOf<UGameplayEffect> ProjectileEffect;	
	
	static FGameplayTag GetShootTag();
	
	//等待BasicAttackTag映射按下
	UFUNCTION()
	void StartShooting(FGameplayEventData PayLoad);
	
	//等价BasicAttackTag映射抬起
	UFUNCTION()
	void StopShooting(FGameplayEventData PayLoad);

	//生成投射物并发射
	UFUNCTION()
	void ShootProjectile(FGameplayEventData PayLoad);

	//获取AimTarget
	AActor* GetAimTargetIfValid() const;

	//利用射线检测找到Target进行射击锁定
	void FindAimTarget();
	
	UPROPERTY()
	AActor* AimTarget;

	UPROPERTY()
	UAbilitySystemComponent* AimTargetAbilitySystemComponent;

	FTimerHandle AimTargetCheckTimerHandle;
	FTimerHandle AimTargetValidTimerHandle;
	
	FVector SpawnLocation;
	
	UPROPERTY(EditDefaultsOnly,Category="Target")
	float AimTargetCheckTimeInterval=0.1f;

	void StartAimTargetCheckTimer();
	void StopAimTargetCheckTimer();
	void StartAimValidCheckTimer();
	void StopAimValidCheckTimer();

	//停止Shoot但是尚未退出Aim状态时，检测Target是否离开视野，如果是则置空Target
	void CheckTargetValidStopShoot();
	
	//判断Target是否合法
	bool HasValidTarget() const ; 

	//判断Target是否在检测范围内
	bool IsTargetInRange() const;

	void TargetDeadTagUpdated(const FGameplayTag Tag,int32 NewCount);
	
	//Montage结束回调
	void OnShootMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	//目标角色是否处于Camera视野内
	bool IsActorInCameraFrustum(AActor* Actor, APlayerController* PlayerController);
	
	bool bInputLocked;
	
	FVector CacheDir;
};
