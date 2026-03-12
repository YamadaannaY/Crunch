
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
	//设置ID并调用皮肤选择函数
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	//判断AI是否具有DeadTag
	bool IsActive() const ;
	
	//移除DeadTag
	void Activate();
	
	//为Minion确定一个Goal进行MoveTo
	void SetGoal(AActor* Goal) const ;

private:
	
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	TMap<FGenericTeamId,USkeletalMesh*> SkinMap;

	UPROPERTY(EditDefaultsOnly,Category="AI")
	FName GoalBlackBoardKeyName="Goal";

	//根据SkinMap中的映射关系根据TeamID选择配置好的Mesh
	void PickSkinBasedOnTeamID();

	//TeamID修改需要重新选择Skin
	virtual void OnRep_TeamID() override;
};
