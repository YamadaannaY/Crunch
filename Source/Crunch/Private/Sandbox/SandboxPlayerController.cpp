// SandboxPlayerController 实现

#include "Sandbox/SandboxPlayerController.h"
#include "Player/CPlayerCharacter.h"
#include "Widgets/GameplayWidget.h"
#include "Net/UnrealNetwork.h"

void ASandboxPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	// 服务端初始化角色 ASC
	CPlayerCharacter = Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		CPlayerCharacter->ServerSideInit();
		CPlayerCharacter->SetGenericTeamId(TeamID);
	}
}

void ASandboxPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);

	CPlayerCharacter = Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		// 客户端初始化 ASC（只设置 ActorInfo）
		CPlayerCharacter->ClientSideInit();

		// 生成 HUD（血条/蓝条/技能列表/属性面板/准星）
		SpawnGameplayWidget();
	}
}

// ──── Team ───────────────────────────────────────────────────

void ASandboxPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId ASandboxPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void ASandboxPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASandboxPlayerController, TeamID);
}

// ──── UI ─────────────────────────────────────────────────────

void ASandboxPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController()) return;
	if (!GameplayWidgetClass) return;

	GameplayWidget = CreateWidget<UGameplayWidget>(this, GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();

		// 绑定角色技能列表到 UI
		if (CPlayerCharacter)
		{
			GameplayWidget->ConfigureAbilities(CPlayerCharacter->GetAbilities());
		}
	}
}
