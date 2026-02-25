#include "LobbyGameMode.h"

ALobbyGameMode::ALobbyGameMode()
{
	//无缝切换关卡，意义在于不会断开客户端连接，GameInstance、PlayerController、PlayerState被保留
	bUseSeamlessTravel = true;
}
