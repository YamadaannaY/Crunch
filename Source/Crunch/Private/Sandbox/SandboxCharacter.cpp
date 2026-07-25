// SandboxCharacter 实现 —— 极简子类，所有逻辑继承自 ACPlayerCharacter

#include "Sandbox/SandboxCharacter.h"

ASandboxCharacter::ASandboxCharacter()
{
	// 构造完全继承 ACPlayerCharacter：
	// - CameraBoom + ViewCamera（第三人称相机）
	// - CAbilitySystemComponent + CAttributeSet + CHeroAttributeSet
	// - MotionWarpingComponent
	// - InventoryComponent（存在但不活跃，无 Shop 入口）
	// - OverHeadWidgetComponent
	// - 二段跳 / 冲刺 / Aim 配置
	//
	// 无需额外初始化 —— 一切由父类处理
}
