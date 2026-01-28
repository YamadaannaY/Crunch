// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_GrounPick.generated.h"

/**
 * 
 */
UCLASS()
class ATargetActor_GroundPick : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	ATargetActor_GroundPick();
	
	void SetTargetAreaRadius(float NewRadius);
	FORCEINLINE void SetTargetTraceRange(float NewRange) {TargetTraceRange=NewRange;};

	//只在客户端调用的函数，接受Confirm输入后执行Overlap范围检测，获取要作用的所有目标对象并广播Confirm
	virtual void ConfirmTargetingAndContinue() override;

	//用于设置此GA影响目标群体
	void SetTargetOptions(bool bTargetFriendly,bool bTargetEnemy=true);

	FORCEINLINE void SetShouldDrawDebug(bool bDrawDebug) {bShouldDrawDebug=bDrawDebug;}

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
