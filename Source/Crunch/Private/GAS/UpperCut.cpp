// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UpperCut.h"

#include "GameplayTagsManager.h"
#include "GA_Combo.h"
#include "UCAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UUpperCut::UUpperCut()
{
	//防止BasicAttack中断UpperCut
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

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
	//初始化为None
	NextComboName=NAME_None;
}

FGameplayTag UUpperCut::GetUpperCutLaunchTag()
{
	//Montage中调用，此时实现Launch效果
	return FGameplayTag::RequestGameplayTag("ability.uppercut.launch");
}

const FGenericDamageEffectDef* UUpperCut::GetDamageEffectDefForCurrentCombo() const
{
	UAnimInstance* OwnAnimInstance=GetOwnerAnimInstance();
	if (OwnAnimInstance)
	{
		//获得当前MontageSectionName
		const FName CurrentComboName=GetOwnerAnimInstance()->Montage_GetCurrentSection(UpperCutMontage);

		//根据Section找到Map中对应的映射
		const FGenericDamageEffectDef* EffectDef=ComboDamageMap.Find(CurrentComboName);

		return EffectDef;
	}
	return nullptr;
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

	//处理ComboChange时的逻辑，当Montage中出发到带有ChangeTag的Notify时调用，用于为NextComboName赋值
	UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UGA_Combo::GetComboChangedEventTag(),nullptr,false,false);
	WaitComboChangeEvent->EventReceived.AddDynamic(this,&ThisClass::HandleComboChangeEvent);
	WaitComboChangeEvent->ReadyForActivation();

	//处理ComboChange后对SectionName赋值结果的处理
	UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UCAbilitySystemStatics::GetBasicAttackInputPressedTag());
	WaitComboCommitEvent->EventReceived.AddDynamic(this,&ThisClass::HandleComboCommitEvent);
	WaitComboCommitEvent->ReadyForActivation();

	//处理Damage逻辑
	UAbilityTask_WaitGameplayEvent* WaitComboDamageEvent=UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,UGA_Combo::GetComboTargetEventTag());
	WaitComboDamageEvent->EventReceived.AddDynamic(this,&ThisClass::HandleComboDamageEvent);
	WaitComboDamageEvent->ReadyForActivation();
}

void UUpperCut::HandleComboChangeEvent(FGameplayEventData EventData)
{
	FGameplayTag EventTag=EventData.EventTag;
	if (EventTag==UGA_Combo::GetComboChangedEventEndTag())
	{
		NextComboName=NAME_None;
		UE_LOG(LogTemp,Warning,TEXT("next combo is cleared"));
		return ;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag,TagNames);
	NextComboName=TagNames.Last();
	UE_LOG(LogTemp,Warning,TEXT("next combo is:%s"),*NextComboName.ToString());
}

void UUpperCut::HandleComboCommitEvent(FGameplayEventData EventData)
{
	//当前Notify为EndNotify
	if (NextComboName==NAME_None) return;
	
	UAnimInstance* OwnerAnimInst=GetOwnerAnimInstance();
	if (!OwnerAnimInst) return;

	//设置当前MontageSection的NextSection为当前遍历Tag的SetcionName
	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(UpperCutMontage),NextComboName,UpperCutMontage);
}

void UUpperCut::HandleComboDamageEvent(FGameplayEventData EventData)
{
	//在服务端执行
	if (K2_HasAuthority())
	{
		//获得Sweep轨迹记录的HitResult数组
		TArray<FHitResult> TargetHitResult=GetHitResultsFromSweepLocationTargetData(EventData.TargetData,TargetSweepSphereRadius,ETeamAttitude::Hostile,ShouldDrawDebug());

		//自身施加向上击飞效果
		PushTarget(GetAvatarActorFromActorInfo(),FVector::UpVector*UpperComboHoldSpeed);

		const FGenericDamageEffectDef* EffectDef=GetDamageEffectDefForCurrentCombo();
		if (!EffectDef) return;
		//所有Hit对象施加向上击飞效果
		for (FHitResult& HitResult : TargetHitResult)
		{
			//TransformVector：把角色自身坐标系里的方向，旋转到世界坐标系，也就是PushVel的结果是在角色当前坐标系内PushVelocity的xyz值应用于世界内
			//eg：（FVector(1000, 0, 200);） 即PushVel：向角色前方推1000，向上推200
			//可优化：可以改为按照ImpactNormal作为PushVel
			FVector PushVel=GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(EffectDef->PushVelocity);

			//将这个Vector施加于对象
			PushTarget(HitResult.GetActor(),PushVel);

			//对HitResult应用DamageGE
			ApplyGameplayEffectToHitResultActor(HitResult,EffectDef->DamageEffect,GetAbilityLevel(CurrentSpecHandle,CurrentActorInfo));
		}
	}
}

