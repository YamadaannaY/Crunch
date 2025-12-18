// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_GroundBlast.h"
#include "UCAbilitySystemStatics.h"
#include "GAS/TargetActor_GrounPick.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

UGA_GroundBlast::UGA_GroundBlast()
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_GroundBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

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
	WaitTargetDataTask->FinishSpawningActor(this,TargetActor);
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Handle)
{
}

void UGA_GroundBlast::TargetCancelled(const FGameplayAbilityTargetDataHandle& Handle)
{
}



