// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameplayAbility.h"

#include "Kismet/KismetSystemLibrary.h"

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
	const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, bool bShowDebug,
	bool bIgnoreSelf)
{
	//输出的碰撞结果数组
	TArray<FHitResult> OutResults;

	TArray<AActor*> HitActors;

	for (const TSharedPtr<FGameplayAbilityTargetData> TargetData : TargetDataHandle.Data)
	{
		FVector StartLoc=TargetData->GetOrigin().GetTranslation();
		FVector EndLoc=TargetData->GetEndPoint();

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
			HitActors.Add(Result.GetActor());
			OutResults.Add(Result);
		}
	}

	return OutResults;
}
