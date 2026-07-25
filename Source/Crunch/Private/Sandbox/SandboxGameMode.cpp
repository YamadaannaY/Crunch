// SandboxGameMode 实现：极简角色测试环境

#include "Sandbox/SandboxGameMode.h"
#include "Sandbox/SandboxPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

ASandboxGameMode::ASandboxGameMode()
{
	// 默认使用沙盒 PlayerController（可在蓝图子类中覆盖）
	PlayerControllerClass = ASandboxPlayerController::StaticClass();
}

APlayerController* ASandboxGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	// 生成 SandboxPlayerController（代替主模块的 ACPlayerController）
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);

	// 可选的队伍分配
	IGenericTeamAgentInterface* TeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	if (TeamInterface)
	{
		const FGenericTeamId TeamID = GetTeamIDForPlayer();
		TeamInterface->SetGenericTeamId(TeamID);

		// 根据 TeamID 查找出生点
		NewPlayerController->StartSpot = FindNextStartSpotTeam(TeamID);
	}

	return NewPlayerController;
}

UClass* ASandboxGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// 直接返回配置的测试 Pawn，不查找 PlayerState 英雄选择数据
	if (DefaultTestPawn)
	{
		return DefaultTestPawn;
	}

	// 回退到 GameMode 基类的默认 PawnClass（可在蓝图 GameMode 中设置）
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

FGenericTeamId ASandboxGameMode::GetTeamIDForPlayer() const
{
	// 简单轮转：玩家 1 → Team 0, 玩家 2 → Team 1, 玩家 3 → Team 0...
	++PlayerCount;
	return FGenericTeamId(PlayerCount % 2);
}

AActor* ASandboxGameMode::FindNextStartSpotTeam(const FGenericTeamId TeamID) const
{
	// 未配置队伍出生点映射 → 使用默认 PlayerStart
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamID);
	if (!StartSpotTag)
	{
		// 返回第一个未被占用的 PlayerStart
		const UWorld* World = GetWorld();
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			if (It->PlayerStartTag != FName("Taken"))
			{
				It->PlayerStartTag = FName("Taken");
				return *It;
			}
		}
		return nullptr;
	}

	// 根据 Tag 查找对应出生点
	const UWorld* World = GetWorld();
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}

	return nullptr;
}
