//在LobbyMap中配置了LobbyPlayerController，每一个角色都拥有一个LobbyMap，通过LobbyGameModeLobbyWidget


#pragma once

#include "CoreMinimal.h"
#include "MenuPlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API ALobbyPlayerController : public AMenuPlayerController
{
	GENERATED_BODY()

public:
	//在Widget引起的逻辑中被调用，所以在客户端中，因此需要一个RPC，将被点击Slot的ID发送给GameState
	UFUNCTION(Server ,Reliable , WithValidation)
	void Server_RequestSlotSelectionChange(uint8 NewSlotID);
};
