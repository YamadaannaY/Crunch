// Fill out your copyright notice in the Description page of Project Settings.


#include "GAP_Launch.h"
#include "UCAbilitySystemStatics.h"

UGAP_Launch::UGAP_Launch()
{
	//PassiveGA期望只在服务端执行
	NetExecutionPolicy=EGameplayAbilityNetExecutionPolicy::ServerOnly;

	//收到一个GameplayEvent，且事件的EventTag==TriggerTag时这个GA才会被触发。
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource=EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag=GetLaunchedAbilityActivationTag();

	ActivationBlockedTags.RemoveTag(UCAbilitySystemStatics::GetStunStatTag());
	
	//添加到Triggers中
	AbilityTriggers.Add(TriggerData);
}

void UGAP_Launch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (K2_HasAuthority())
	{
		//GA只在服务端触发：具体逻辑就是对自己施加一个特定方向的速度，这个速度就是PushTarget函数中传参的PushVel
		PushSelf(TriggerEventData->TargetData.Get(0)->GetHitResult()->ImpactNormal);
		K2_EndAbility();
	}
}

FGameplayTag UGAP_Launch::GetLaunchedAbilityActivationTag()
{
	return FGameplayTag::RequestGameplayTag("ability.passive.launch.activate");
}
