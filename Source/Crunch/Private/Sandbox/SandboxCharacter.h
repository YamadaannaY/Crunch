// SandboxCharacter: 角色测试专用 Character
// 继承 ACPlayerCharacter 获得完整功能（移动/相机/跳跃/冲刺/GA/动画/UI），
// 仓库组件仍存在但完全不活跃（无 Shop UI 入口，无物品来源）
//
// 属于 Crunch 模块（同模块编译，无需 API 导出宏）

#pragma once

#include "CoreMinimal.h"
#include "Player/CPlayerCharacter.h"
#include "SandboxCharacter.generated.h"

/**
 * 沙盒测试角色 —— 极简子类，不添加任何额外逻辑。
 *
 * ✅ 完整保留：
 *    - GAS 能力系统（ASC + AttributeSet + 所有 GA）
 *    - 属性集（CAttributeSet + CHeroAttributeSet）
 *    - 动画（CAnimInstance + HitReact + Death/Ragdoll）
 *    - 移动 / 相机 / 二段跳 / 冲刺 / Aim / Focus
 *    - OverHeadUI / 武器粒子特效 / AI 感知
 *    - 学习/升级能力 (Leader + Input 组合键)
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
 *   2. 在蓝图中不配置 UseInventoryItemAction（仓库输入动作留空即可）
 *   3. 配置 PA_AbilitySystemGeneric（属性表 + 初始 GE + 能力）
 *   4. 在 SandboxGameMode 中将 DefaultTestPawn 设为 BP_SandboxCharacter
 */
UCLASS()
class ASandboxCharacter : public ACPlayerCharacter
{
	GENERATED_BODY()

public:
	ASandboxCharacter();
};
