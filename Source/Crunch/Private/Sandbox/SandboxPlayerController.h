// SandboxPlayerController: 精简的 PlayerController
// 复用主模块的 UGameplayWidget（血条/蓝条/技能列表/准星），但移除商店/仓库/MatchFinished 逻辑
//
// 属于 Crunch 模块（同模块编译，无需 API 导出宏）

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "SandboxPlayerController.generated.h"

class UGameplayWidget;
class ACPlayerCharacter;

/**
 * 沙盒 PlayerController：
 * - 保留 GameplayWidget（Health/Mana/AbilityList/Crosshair/StatsGauge）
 * - 移除 Shop/Menu 切换输入
 * - 移除 MatchFinished 逻辑
 * - 移除 InventoryContextMenu 清除
 * - 保留 TeamID 复制（用于队伍相关能力测试）
 */
UCLASS()
class ASandboxPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// ──── Possess ────────────────────────────────────────────

	virtual void OnPossess(APawn* NewPawn) override;
	virtual void AcknowledgePossession(APawn* NewPawn) override;

	// ──── Team ───────────────────────────────────────────────

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// ──── UI ─────────────────────────────────────────────────

	// 本地客户端生成 GameplayWidget（血条/蓝条/技能列表/属性面板/准星）
	void SpawnGameplayWidget();

	// GamplayWidget 蓝图类
	UPROPERTY(EditDefaultsOnly, Category = "Sandbox|UI")
	TSubclassOf<UGameplayWidget> GameplayWidgetClass;

	// 运行时 GameplayWidget 实例
	UPROPERTY()
	TObjectPtr<UGameplayWidget> GameplayWidget;

private:
	// 当前控制的角色（缓存用于 UI 绑定）
	UPROPERTY()
	TObjectPtr<ACPlayerCharacter> CPlayerCharacter;

	// 复制给客户端的 TeamID
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;
};
