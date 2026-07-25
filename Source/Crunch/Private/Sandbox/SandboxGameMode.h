// SandboxGameMode: 极简 GameMode，只负责生成 Controller 和角色
// 移除了 StormCore（推车胜负）、CGameState（英雄选择）、MinionBarrack（AI兵营）、
// 仓库/商店 等主模块玩法逻辑，仅保留角色生成和可选队伍分配
//
// 属于 Crunch 模块（同模块编译，无需 API 导出宏）

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "SandboxGameMode.generated.h"

/**
 * 沙盒 GameMode：用于角色开发与能力测试
 * - 不依赖 AStormCore（无推车胜负逻辑）
 * - 不依赖 ACGameState（无英雄选择数据）
 * - 不依赖 AMinionBarrack（无 AI 兵营）
 * - 不依赖 UCAssetManager 配方系统
 * - 保留简单的队伍分配（可选，用于测试队伍相关能力）
 */
UCLASS()
class ASandboxGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASandboxGameMode();

	// 服务端生成 PlayerController，分配 TeamID 和出生点
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

	// 返回沙盒测试角色的 PawnClass
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

private:
	// 简单的轮流分配队伍（0, 1, 0, 1...）
	FGenericTeamId GetTeamIDForPlayer() const;

	// 根据 TeamID 查找对应的 PlayerStart
	AActor* FindNextStartSpotTeam(const FGenericTeamId TeamID) const;

	// ──── 可配置属性 ────────────────────────────────────────

	// TeamID → PlayerStart 的 Tag 映射（如 0→"TeamA", 1→"TeamB"）
	// 留空则使用默认 PlayerStart 生成
	UPROPERTY(EditDefaultsOnly, Category = "Sandbox|Team")
	TMap<FGenericTeamId, FName> TeamStartSpotTagMap;

	// 测试用的默认 Pawn 类（在蓝图/编辑器中配置为你的 BP_SandboxCharacter）
	UPROPERTY(EditDefaultsOnly, Category = "Sandbox|Character")
	TSubclassOf<APawn> DefaultTestPawn;

	// 内部轮转计数器
	mutable int PlayerCount = 0;
};
