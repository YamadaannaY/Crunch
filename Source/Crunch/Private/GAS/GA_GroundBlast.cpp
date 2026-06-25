#include "GA_GroundBlast.h"
#include "GAS/TargetActor_GrounPick.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GenericTeamAgentInterface.h"

UGA_GroundBlast::UGA_GroundBlast():GroundBlastMontage(nullptr),CastMontage(nullptr)
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_GroundBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	
	if (!HasAuthorityOrPredictionKey(CurrentActorInfo,&CurrentActivationInfo)) return ;

	UAbilityTask_PlayMontageAndWait* PlayGroundBlastAnimTask=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,GroundBlastMontage);
	PlayGroundBlastAnimTask->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
	PlayGroundBlastAnimTask->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
	PlayGroundBlastAnimTask->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
	PlayGroundBlastAnimTask->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
	PlayGroundBlastAnimTask->ReadyForActivation();

	//Confirm和Cancel分别触发两个回调，对应了此GA的两个选择，是否释放此技能
	UAbilityTask_WaitTargetData* WaitTargetDataTask=UAbilityTask_WaitTargetData::WaitTargetData(this,NAME_None,EGameplayTargetingConfirmation::UserConfirmed,TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this,&ThisClass::TargetConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this,&ThisClass::TargetCancelled);
	WaitTargetDataTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	//生成TargetActor
	WaitTargetDataTask->BeginSpawningActor(this,TargetActorClass,TargetActor);
	
	ATargetActor_GroundPick* GroundPickActor=Cast<ATargetActor_GroundPick>(TargetActor);

	//对检测区域进行配置
	if (GroundPickActor)
	{
		GroundPickActor->SetShouldDrawDebug(ShouldDrawDebug());
		GroundPickActor->SetTargetAreaRadius(TargetAreaRadius);
		GroundPickActor->SetTargetTraceRange(TargetTraceRange);
	}
	
	WaitTargetDataTask->FinishSpawningActor(this,TargetActor);
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Handle)
{
	//由于不是GA激活就直接进行Cost/Cooldown，需要在实际作用时手动Commit
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return ;
	}

	//TA只提供瞄准坐标(Index 0)，服务端据此做权威Overlap选择有效目标
	const FVector ImpactPoint=UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Handle,0).ImpactPoint;

	if(K2_HasAuthority())
	{
		const IGenericTeamAgentInterface* OwnerTeamAgent=Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
		TArray<AActor*> ValidTargets=ATargetActor_GroundPick::GetValidTargetsAtLocation(
			GetWorld(),ImpactPoint,TargetAreaRadius,OwnerTeamAgent,false,true);

		FGameplayAbilityTargetDataHandle AuthoritativeHandle=UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(ValidTargets,false);
		BP_ApplyGameplayEffectToTarget(AuthoritativeHandle,DamageEffectDef.DamageEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
		PushTargets(AuthoritativeHandle,DamageEffectDef.PushVelocity);
	}

	//CueParam
	FGameplayCueParameters BlastingCueParams;
	BlastingCueParams.Location=ImpactPoint;
	BlastingCueParams.RawMagnitude=TargetAreaRadius;

	//GC所有客户端可见
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BlastGameplayCueTag,BlastingCueParams);
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(UCAbilitySystemStatics::GetCameraShakeCueTag(),BlastingCueParams);

	UAnimInstance* OwnerAnim=GetOwnerAnimInstance();
	if (OwnerAnim)
	{
		//此时播放一个完整的CastMontage，补上后续的手臂下落Montage动画
		OwnerAnim->Montage_Play(CastMontage);
	}

	K2_EndAbility();
}

void UGA_GroundBlast::TargetCancelled(const FGameplayAbilityTargetDataHandle& Handle)
{
	K2_EndAbility();
}