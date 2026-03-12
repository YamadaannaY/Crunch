#include "LobbyGameMode.h"

ALobbyGameMode::ALobbyGameMode()
{
	//无缝切换关卡，意义在于不会断开客户端连接APlayerController、PlayerState等玩家信息被保留
	bUseSeamlessTravel = true;
}
