// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CCharacter.h"
#include "Minion.generated.h"

/**
 * 
 */
UCLASS()
class AMinion : public ACCharacter
{
	GENERATED_BODY()
public:
	//设置ID并
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	//判断AI是否具有DeadTag
	bool IsActive() const ;
	
	//激活AI，即移除DeadTag
	void Activate();
	
	//为Minion确定一个Goal
	void SetGoal(AActor* Goal);

private:
	//根据SkinMap中的映射关系以及自己的TeamID选择Mesh
	void PickSkinBasedOnTeamID();
	
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	TMap<FGenericTeamId,USkeletalMesh*> SkinMap;

	UPROPERTY(EditDefaultsOnly,Category="AI")
	FName GoalBlackBoardKeyName="Goal";

	virtual void OnRep_TeamID() override;
};
