// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UpperCut.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void UUpperCut::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo,&ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayUpperCutMontageTask=UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,UpperCutMontage);
		PlayUpperCutMontageTask->OnBlendOut.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayUpperCutMontageTask->OnCompleted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayUpperCutMontageTask->OnCancelled.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayUpperCutMontageTask->OnInterrupted.AddDynamic(this,&ThisClass::K2_EndAbility);
		PlayUpperCutMontageTask->ReadyForActivation();

		//播放Montage后实现击飞效果
		UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,GetUpperCutLaunchTag());
		WaitLaunchEventTask->EventReceived.AddDynamic(this,&ThisClass::StartLaunching);
		WaitLaunchEventTask->ReadyForActivation();
	}
}

FGameplayTag UUpperCut::GetUpperCutLaunchTag()
{
	//Montage中调用，此时实现Launch效果
	return FGameplayTag::RequestGameplayTag("ability.uppercut.launch");
}

void UUpperCut::StartLaunching(FGameplayEventData EventData)
{
	//在服务端执行
	if (K2_HasAuthority())
	{
		//获得Sweep轨迹记录的HitResult数组
		TArray<FHitResult> TargetHitResult=GetHitResultsFromSweepLocationTargetData(EventData.TargetData,TargetSweepSphereRadius,ETeamAttitude::Hostile,ShouldDrawDebug());

		//自身施加向上击飞效果
		PushTarget(GetAvatarActorFromActorInfo(),FVector::UpVector*UpperCutLaunchSpeed);

		//所有Hit对象施加向上击飞效果
		for (FHitResult& HitResult : TargetHitResult)
		{
			PushTarget(HitResult.GetActor(),FVector::UpVector*UpperCutLaunchSpeed);

			ApplyGameplayEffectToHitResultActor(HitResult,LaunchDamageEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
		}
	}
}

