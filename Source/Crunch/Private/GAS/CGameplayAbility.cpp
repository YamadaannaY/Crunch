// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAP_Launch.h"
#include "UCAbilitySystemStatics.h"
#include "Character/CCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

UCGameplayAbility::UCGameplayAbility()
{
	ActivationBlockedTags.AddTag(UCAbilitySystemStatics::GetStunStatTag());
}

bool UCGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	FGameplayAbilitySpec* AbilitySpec=ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);

	//没有GA或者GA是0级不能激活
	if (AbilitySpec && AbilitySpec->Level<=0) return false;

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

UAnimInstance* UCGameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* OwnerSkeletalMeshComp=GetOwningComponentFromActorInfo();
	if (OwnerSkeletalMeshComp)
	{
		return OwnerSkeletalMeshComp->GetAnimInstance();
	}
	return nullptr;
}

TArray<FHitResult> UCGameplayAbility::GetHitResultsFromSweepLocationTargetData(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, ETeamAttitude::Type TargetTeam,bool bShowDebug,
	bool bIgnoreSelf)
{
	//输出的碰撞结果数组
	TArray<FHitResult> OutResults;
	//记录HitActor以避免重复记录同一目标的Result
	TArray<AActor*> HitActors;
	
	const IGenericTeamAgentInterface* OwnerTeamInterface=Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	
	for (const TSharedPtr<FGameplayAbilityTargetData> TargetData : TargetDataHandle.Data)
	{
		const FVector StartLoc=TargetData->GetOrigin().GetTranslation();
		const FVector EndLoc=TargetData->GetEndPoint();

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectType;
		ObjectType.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		TArray<AActor*> ActorsToIgnore;
		
		if (bIgnoreSelf)
		{
			ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
		}

		EDrawDebugTrace::Type DrawDebugTrace=bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

		TArray<FHitResult> Results;
		
		UKismetSystemLibrary::SphereTraceMultiForObjects(this,StartLoc,EndLoc,SphereSweepRadius,ObjectType,false,ActorsToIgnore,DrawDebugTrace,Results,false);

		//一次攻击不重复对Actor造成伤害
		for (const FHitResult& Result : Results)
		{
			if (HitActors.Contains(Result.GetActor()))
			{
				continue;
			}
			
			//如果碰撞对象的TeamAttitude不是Hostile，忽略
			if (OwnerTeamInterface)
			{
				const ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*Result.GetActor());
				if (OtherActorTeamAttitude != TargetTeam) continue;
			}
			HitActors.Add(Result.GetActor());
			OutResults.Add(Result);
		}
	}
	return OutResults;
}

void UCGameplayAbility::PushSelf(const FVector& PushVel)
{
	ACharacter* OwningAvatarCharacter=GetOwningAvatarCharacter();
	if (OwningAvatarCharacter)
	{
		OwningAvatarCharacter->LaunchCharacter(PushVel,true,true);
	}
}

ACharacter* UCGameplayAbility::GetOwningAvatarCharacter()
{
	if (!AvatarCharacter)
	{
		AvatarCharacter=Cast<ACharacter>(GetAvatarActorFromActorInfo());
	}
	return AvatarCharacter;
}

void UCGameplayAbility::PushTarget(AActor* Target, const FVector& PushVel)
{
	if (!Target) return;
	
	FGameplayEventData EventData;

	FGameplayAbilityTargetData_SingleTargetHit* HitData=new FGameplayAbilityTargetData_SingleTargetHit;
	FHitResult HitResult;
	HitResult.ImpactNormal=PushVel;
	
	HitData->HitResult=HitResult;
	EventData.TargetData.Add(HitData);

	//PassiveGA中设置了以GameplayEvent+EventTag，这里直接触发
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target,UGAP_Launch::GetLaunchedAbilityActivationTag(),EventData);
}

void UCGameplayAbility::PushTargets(const TArray<AActor*>& Targets, const FVector PushVel)
{
	for (AActor* Target : Targets)
	{
		PushTarget(Target,PushVel);
	}
}

void UCGameplayAbility::PushTarget(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& PushVel)
{
	TArray<AActor*> Targets=UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
	PushTargets(Targets,PushVel);
}

void UCGameplayAbility::ApplyGameplayEffectToHitResultActor(const FHitResult HitResult,
                                                            TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	//制作GE的SpecHandle
	FGameplayEffectSpecHandle EffectSpecHandle=MakeOutgoingGameplayEffectSpec(GameplayEffect,Level);

	//特别配置当前GA的EffectContext，用于记录碰撞对象用于GameplayCue
	FGameplayEffectContextHandle EffectContext=MakeEffectContext(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo());
	EffectContext.AddHitResult(HitResult);
	EffectSpecHandle.Data->SetContext(EffectContext);
		
	ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(),CurrentActorInfo,CurrentActivationInfo,EffectSpecHandle,UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
}