// SandboxCharacter: 角色测试专用 Character
// 继承 ACPlayerCharacter 获得完整功能（移动/相机/跳跃/冲刺/GA/动画/UI），
// 仓库组件仍存在但完全不活跃（无 Shop UI 入口，无物品来源）
//
// 属于 Crunch 模块（同模块编译，无需 API 导出宏）

#pragma once

#include "CoreMinimal.h"
#include "Player/CPlayerCharacter.h"
#include "SandboxCharacter.generated.h"

class UInputAction;

/**
 * 沙盒测试角色 —— 在 ACPlayerCharacter 基础上扩展 Walk/Run 切换
 *
 * ✅ 完整保留：
 *    - GAS 能力系统（ASC + AttributeSet + 所有 GA）
 *    - 属性集（CAttributeSet + CHeroAttributeSet）
 *    - 动画（CAnimInstance + HitReact + Death/Ragdoll）
 *    - 移动 / 相机 / 二段跳 / 冲刺 / Aim / Focus
 *    - OverHeadUI / 武器粒子特效 / AI 感知
 *    - 学习/升级能力 (Leader + Input 组合键)
 *
 * ➕ 沙盒扩展：
 *    - Ctrl 点按切换 Walk / Run 模式
 *    - bWalkMode 驱动 SandboxAnimInstance 的 WalkRun 参数
 *
 * ❌ 沙盒中自然不活跃（GameMode/Controller 层切断入口）：
 *    - 仓库（无 Shop 切换、无物品数据）
 *    - StormCore 推车玩法
 *    - AI 兵营（MinionBarrack）
 *    - 英雄选择（CGameState PlayerSelection）
 *    - MatchFinished 结算逻辑
 *
 * 使用方式：
 *   1. 基于此类创建蓝图 BP_SandboxCharacter
 *   2. 蓝图 AnimClass 设置为 USandboxAnimInstance 的派生 BP
 *   3. 配置 CtrlInputAction（EnhancedInput Action，Key=Left Ctrl）
 *   4. 在 SandboxGameMode 中将 DefaultTestPawn 设为 BP_SandboxCharacter
 */
UCLASS()
class ASandboxCharacter : public ACPlayerCharacter
{
	GENERATED_BODY()

public:
	ASandboxCharacter();

	// AnimInstance 通过此接口读取 Walk 状态
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsWalkMode() const { return bWalkMode; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// ──── Walk / Run Toggle ────────────────────────────────

	// 切换 Walk / Run 模式（由 Ctrl 键触发）
	void HandleCtrlToggle(const FInputActionValue& InputActionValue);

	// RPC：服务端切换 WalkMode 并更新 MaxWalkSpeed
	UFUNCTION(Server, Reliable)
	void Server_ToggleWalkMode();

	// 是否处于步行模式（Replicated，供 AnimInstance 读取）
	UPROPERTY(Replicated)
	bool bWalkMode = false;

	// Walk 模式下的 MaxWalkSpeed
	UPROPERTY(EditDefaultsOnly, Category="Movement|WalkRun")
	float WalkSpeed = 300.f;

	// Run 模式下的 MaxWalkSpeed（默认值 600，可在蓝图调整）
	UPROPERTY(EditDefaultsOnly, Category="Movement|WalkRun")
	float RunSpeed = 600.f;

	// Ctrl 键 InputAction（蓝图配置）
	UPROPERTY(EditDefaultsOnly, Category="Movement|WalkRun")
	UInputAction* CtrlInputAction;
};
