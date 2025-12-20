// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GAP_Dead.h"
#include "Engine/OverlapResult.h"
#include "UCAbilitySystemStatics.h"

UGAP_Dead::UGAP_Dead()
{
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
		//击杀者，通过EffectContext获取
		AActor* Killer=TriggerEventData->ContextHandle.GetEffectCauser();
		if (Killer)
		{
			UE_LOG(LogTemp,Warning,TEXT("the killer is:%s"),*Killer->GetName());
		}

		//检测范围内的Reward对象
		TArray<AActor*> RewardTargets=GetRewardTargets();
		for (const AActor* RewardTarget : RewardTargets)
		{
			UE_LOG(LogTemp,Warning,TEXT("find award target is:%s"),*RewardTarget->GetName());
		}
	}
}

TArray<AActor*> UGAP_Dead::GetRewardTargets() const
{
	TSet<AActor*> OutActors;

	AActor* AvatarActor=GetAvatarActorFromActorInfo();

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
		for (const FOverlapResult OverlapResult : OverlapResults)
		{
			IGenericTeamAgentInterface* OtherTeamInterface=Cast<IGenericTeamAgentInterface>(OverlapResult.GetActor());

			//只获取Hero且友方的单位
			if (!OtherTeamInterface || OtherTeamInterface->GetTeamAttitudeTowards(*AvatarActor) ==ETeamAttitude::Hostile)
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
