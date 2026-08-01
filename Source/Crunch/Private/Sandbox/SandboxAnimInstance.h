#pragma once

#include "CoreMinimal.h"
#include "Animations/CAnimInstance.h"
#include "Animations/AN_FootPlant.h"
#include "SandboxAnimInstance.generated.h"

/**
 * SandboxAnimInstance — 沙盒测试专用 AnimInstance
 *
 * 在 CAnimInstance 基础上新增：
 *   - BS_WalkRun_F BlendSpace: Stride (X轴) + WalkRun (Y轴)
 *   - 待机动画随机切换 (Timeline):
 *       常驻 Idle ──Timer 到──▶ 随机 Stand_Action ──播完──▶ 常驻 Idle ──重新计时──▶ ...
 *
 *   AnimBP 中 BlendPosesByInt:
 *     Index 0 = 常驻 Idle
 *     Index 1..N = Stand_Action 变体
 *
 *   每个 Action 通过 StandActionDurations 数组单独配置播放时长，
 *   未覆盖的 Action 回落 DefaultActionDuration。
 */
UCLASS()
class CRUNCH_API USandboxAnimInstance : public UCAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ──── Locomotion ──────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetStride() const { return Stride; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetWalkRun() const { return WalkRun; }

	// ──── Stand Idle ─────────────────────────────────────────────

	// 当前动画索引，接 AnimBP BlendPosesByInt 节点的 Active Child Index
	// 0 = 常驻 Idle，1..N = Stand_Action 变体
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE int32 GetStandIdleIndex() const { return StandIdleIndex; }

	// 是否正在播放 Stand_Action
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsPlayingStandAction() const { return StandIdleIndex > 0; }

	// 获取当前 Action 的时长（供 AnimBP / Debug 查询）
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetCurrentActionDuration() const { return CurrentActionDuration; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE  bool GetWalkMode() const { return bWalkMode; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE  bool HasMovementInput() const { return bHasMovementInput; }
	
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE  bool ShouldEnterLeftStop() const {return EnterLeftStop;}
	

	void OnFootPlantNotify(EFootPlant Foot);
private:
	// ──── Locomotion ──────────────────────────────────────────────
	float Stride = 0.f;
	float WalkRun = 0.f;

	bool bWalkMode = true;
	
	bool EnterLeftStop = true ;

	UPROPERTY(EditAnywhere, Category="Animation|Locomotion")
	float WalkRunInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, Category="Animation|Locomotion", meta=(ClampMin="0.5", ClampMax="1.0"))
	float StrideMaxSpeedScale = 0.9f;

	// ──── Stand Idle State Machine ────────────────────────────────
	//
	//  [ 常驻 Idle ] ──Timer 到──▶ [ Stand_Action ] ──播完──▶ [ 常驻 Idle ]
	//      Index = 0               Index = 1..N                 Index = 0
	//                              Timer 冻结                   Timer 归零重计
	//
	// 角色移动 / 离地 → 立刻切回常驻 Idle，Timer 归零

	int32 StandIdleIndex = 0;
	int32 StandIdlePrevActionIndex = INDEX_NONE;

	// 常驻 Idle 阶段累计时间
	float StandIdleTimer = 0.f;

	// Stand_Action 阶段累计时间
	float StandActionTimer = 0.f;

	// 当前 Action 时长（切换时从 StandActionDurations 读取）
	float CurrentActionDuration = 0.f;

	// ──── 可配属性 ────────────────────────────────────────────────

	// 在常驻 Idle 中停留多久后触发随机 Action
	UPROPERTY(EditAnywhere, Category="Animation|Idle", meta=(ClampMin="1.0", ClampMax="30.0"))
	float StandIdleChangeInterval = 15.f;

	// 每个 Action 的单独时长（秒），索引 0 对应 BlendPose[1]，索引 1 对应 BlendPose[2]...
	// 未覆盖的 Action 回落 DefaultActionDuration
	UPROPERTY(EditAnywhere, Category="Animation|Idle")
	TArray<float> StandActionDurations;

	// 默认 Action 时长 — StandActionDurations 未覆盖时使用
	UPROPERTY(EditAnywhere, Category="Animation|Idle", meta=(ClampMin="0.5", ClampMax="30.0"))
	float DefaultActionDuration = 4.f;

	// Stand_Action 变体数量（不含常驻 Idle）
	// BlendPosesByInt 总 Pin 数 = 1 + StandActionVariantCount
	UPROPERTY(EditAnywhere, Category="Animation|Idle", meta=(ClampMin="1", ClampMax="9"))
	int32 StandActionVariantCount = 3;
	
	UPROPERTY(EditDefaultsOnly,Category="Movement")
	bool bHasMovementInput;
};
