//从摄像机原点和朝向开始进行一次射线检测，如果没找到则向下检测，如果还是没有则保持原点，检测后更新TA位置，并对其周围进行一次射线检测，返回ImpactPoint

#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_GrounPick.generated.h"

class IGenericTeamAgentInterface;
/**
 * 地面瞄准TargetActor：根据玩家相机视角确定瞄准位置。
 * 注意：此TA只负责提供瞄准坐标(ImpactPoint)，不再做目标选择。
 * 目标选择由GA在服务端通过 GetValidTargetsAtLocation 权威执行。
 */
UCLASS()
class ATargetActor_GroundPick : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	ATargetActor_GroundPick();

	void SetTargetAreaRadius(float NewRadius);
	FORCEINLINE void SetTargetTraceRange(float NewRange) {TargetTraceRange=NewRange;};

	//接受Confirm输入，只提供瞄准位置(ImpactPoint)，不选择目标
	virtual void ConfirmTargetingAndContinue() override;

	//用于设置此GA影响目标群体
	void SetTargetOptions(bool bTargetFriendly,bool bTargetEnemy=true);

	FORCEINLINE void SetShouldDrawDebug(bool bDrawDebug) {bShouldDrawDebug=bDrawDebug;}

	//服务端权威Overlap：在指定位置做球体检测，按队伍态度过滤，返回有效目标Actor数组
	static TArray<AActor*> GetValidTargetsAtLocation(
		UWorld* World,
		FVector Location,
		float Radius,
		const IGenericTeamAgentInterface* OwnerTeamAgent,
		bool bTargetFriendly,
		bool bTargetEnemy);

private:
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	UDecalComponent* DecalComp;

	virtual void Tick(float DeltaSeconds) override;

	bool bShouldTargetEnemy=true;
	bool bShouldTargetFriendly=false;
	
	//射线检测，通过HitResult确定TargetActor的位置
	FVector GetTargetPoint()const ;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float TargetTraceRange=2000.f;

	UPROPERTY(EditDefaultsOnly,Category="Targeting")
	float TargetAreaRadius=300.f;

	bool bShouldDrawDebug=false;
};
