/*****************  用于利用多个Socket组成的轨迹进行碰撞检测并返回TargetGroup	************************/


#include "Animations/AN_SendTargetGroup.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return ;

	//1个Socket无法触发Group
	if (TargetSocketName.Num() <=1) return ;

	if (!MeshComp->GetOwner() || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner())) return ;

	//用Data存储Socket的位置
	FGameplayEventData Data;
	TSet<AActor*> HitActors;
	
	AActor* OwnerActor=MeshComp->GetOwner();
	const IGenericTeamAgentInterface* OwnerTeamInterface=Cast<IGenericTeamAgentInterface>(OwnerActor);
	
	for (int i=1;i<TargetSocketName.Num();++i)
	{
		//遍历所有Socket，每两个一组形成一段轨迹以模拟路径
		FVector StratLoc=MeshComp->GetSocketLocation(TargetSocketName[i-1]);
		FVector EndLoc=MeshComp->GetSocketLocation(TargetSocketName[i]);

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
		
		TArray<FHitResult> HitResults;

		TArray<AActor*> ActorsToIgnore;
		if (bIgnoreSelf)
		{
			ActorsToIgnore.Add(OwnerActor);
		}

		EDrawDebugTrace::Type DrawDebugTrace=bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		
		UKismetSystemLibrary::SphereTraceMultiForObjects(MeshComp,StratLoc,EndLoc,SphereSweepRadius,
			ObjectTypes,false,ActorsToIgnore,DrawDebugTrace,HitResults,false);

		for (const FHitResult& HitResult : HitResults)
		{
			if (HitActors.Contains(HitResult.GetActor()))
			{
				continue;
			}
			if (OwnerTeamInterface)
			{
				if (OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult.GetActor()) !=TargetTeam)
				{
					continue;
				}
			}
			FGameplayAbilityTargetData_SingleTargetHit* TargetHit=new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
			Data.TargetData.Add(TargetHit);
			SendLocalGameplayCue(HitResult);
		}
	}
	//让EventTag对应的Event获得Socket的SourceLocation和TargetLocation，执行伤害逻辑
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(),EventTag,Data);
}

void UAN_SendTargetGroup::SendLocalGameplayCue(const FHitResult& HitResult) const
{
	FGameplayCueParameters CueParams;
	CueParams.Location=HitResult.ImpactPoint;
	CueParams.Normal=HitResult.ImpactNormal;

	for (const FGameplayTag& GameplayCueTag : TriggerGameplayCueTag)
	{
		//local execute cue
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(HitResult.GetActor(),GameplayCueTag,EGameplayCueEvent::Executed,CueParams);
	}
}
