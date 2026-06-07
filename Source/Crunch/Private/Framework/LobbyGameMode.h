#pragma once

#include "CoreMinimal.h"
#include "CGameMode.h"
#include "LobbyGameMode.generated.h"

/**
 *LobbyGameMode不处理逻辑，纯粹的UI交互，只执行无缝切换关卡  
 */
UCLASS()
class CRUNCH_API ALobbyGameMode : public ACGameMode
{
	GENERATED_BODY()

public:
	ALobbyGameMode();
};
