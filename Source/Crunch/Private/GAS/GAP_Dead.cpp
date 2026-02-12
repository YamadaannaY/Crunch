	// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GAP_Dead.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CHeroAttributeSet.h"
#include "Engine/OverlapResult.h"
#include "UCAbilitySystemStatics.h"

UGAP_Dead::UGAP_Dead()
{
	//PassiveGA只执行GE之类的，只在Server调用即可
	NetExecutionPolicy=EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource=EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag=UCAbilitySystemStatics::GetDeadStatTag();
	AbilityTriggers.Add(TriggerData);
	
	ActivationBlockedTags.RemoveTag(UCAbilitySystemStatics::GetStunStatTag());
}

void UGAP_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (K2_HasAuthority())
	{
		AActor* Killer=TriggerEventData->ContextHandle.GetEffectCauser();
		if (!Killer || !UCAbilitySystemStatics::IsHero(Killer))
		{
			Killer=nullptr;
		}

		//检测范围内的Reward对象
		TArray<AActor*> RewardTargets=GetRewardTargets();

		//单位死亡但不进行奖励结算的情况：Killer和RewardTarget都不存在，如果只有Killer存在则说明单位被Killer全局技能击杀，附近没有其他Killer友方单位，所以
		//只奖励Killer，如果只有RewardTarget存在说明单位是因为其他因素死亡，也会结算奖励

		if (RewardTargets.Num()==0 && !Killer)
		{
			K2_EndAbility();
			return;
		}

		if (Killer && !RewardTargets.Contains(Killer))
		{
			//被远程技能击杀，添加Killer
			RewardTargets.Add(Killer);
		}

		bool bFound=false;
		
		//被击杀者的经验值
		const float SelfExperience=GetAbilitySystemComponentFromActorInfo_Ensured()->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(),bFound);

		//总奖励值计算
		float TotalExperienceReward=BaseExperienceReward+ExperienceRewardPerExperience*SelfExperience;
		float TotalGoldReward=BaseGoldReward+GoldRewardPerExperience*SelfExperience;

		if (Killer)
		{
			//获得指定占比的奖励
			const float KillerExperienceReward=TotalExperienceReward*KillerRewardPortion;
			const float KillerGoldReward=TotalGoldReward*KillerRewardPortion;

			//利用SetByCaller，在代码中即可修改值,这个值在编辑器中设置SetByCaller后根据ModOp修改属性值
			FGameplayEffectSpecHandle EffectSpec=MakeOutgoingGameplayEffectSpec(RewardEffect);
			EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetExperienceAttributeTag(),KillerExperienceReward);
			EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetGoldAttributeTag(),KillerGoldReward);
			
			K2_ApplyGameplayEffectSpecToTarget(EffectSpec,UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));

			TotalExperienceReward-=KillerExperienceReward;
			TotalGoldReward-=KillerGoldReward;
		}

		//对助攻者，进行平均分配
		const float ExperiencePerTarget=TotalExperienceReward/RewardTargets.Num();
		const float GoldPerTarget=TotalGoldReward/RewardTargets.Num();

		const FGameplayEffectSpecHandle EffectSpec=MakeOutgoingGameplayEffectSpec(RewardEffect);
		EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetExperienceAttributeTag(),ExperiencePerTarget);
		EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetGoldAttributeTag(),GoldPerTarget);

		K2_ApplyGameplayEffectSpecToTarget(EffectSpec,UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(RewardTargets,true));

		K2_EndAbility();
	}
}

TArray<AActor*> UGAP_Dead::GetRewardTargets() const
{
	TSet<AActor*> OutActors;

	const AActor* AvatarActor=GetAvatarActorFromActorInfo();

	if (!AvatarActor || !GetWorld())
	{
		return OutActors.Array();
	}

	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(RewardRange);

	TArray<FOverlapResult> OverlapResults;
	if (GetWorld()->OverlapMultiByObjectType(OverlapResults,AvatarActor->GetActorLocation(),FQuat::Identity,QueryParams,CollisionShape))
	{
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			IGenericTeamAgentInterface* OtherTeamInterface=Cast<IGenericTeamAgentInterface>(OverlapResult.GetActor());

			//只获取Hero且友方的单位
			if (!OtherTeamInterface || OtherTeamInterface->GetTeamAttitudeTowards(*AvatarActor) !=ETeamAttitude::Hostile)
			{
				continue;
			}

			if (!UCAbilitySystemStatics::IsHero(OverlapResult.GetActor()))
			{
				continue;
			}
			OutActors.Add(OverlapResult.GetActor());
		}
	}

	return OutActors.Array();
}