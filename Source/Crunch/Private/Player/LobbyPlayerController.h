//在LobbyMap中配置了LobbyPlayerController，每一个角色都拥有一个LobbyMap，通过LobbyGameModeLobbyWidget

#pragma once

#include "CoreMinimal.h"
#include "MenuPlayerController.h"
#include "LobbyPlayerController.generated.h"

DECLARE_DELEGATE(FOnSwitchToHeroSelection);
/**
 * 
 */

UCLASS()
class CRUNCH_API ALobbyPlayerController : public AMenuPlayerController
{
	GENERATED_BODY()

public:
	FOnSwitchToHeroSelection OnSwitchToHeroSelection;
	
	//在Widget引起的逻辑中被调用，所以在客户端中，因此需要一个RPC，将被点击Slot的ID发送给GameState
	UFUNCTION(Server ,Reliable , WithValidation)
	void Server_RequestSlotSelectionChange(uint8 NewSlotID);

	//服务端遍历所有PlayerState并对其Cast然后调用此函数处理对所有客户端的逻辑
	UFUNCTION(Server,Reliable,WithValidation)
	void Server_StartHeroSelection();

	//在客户端广播委托，触发绑定的回调函数，主要是通过Switcher进行Widget切换
	UFUNCTION(Client,Reliable)
	void ClientStartHeroSelection();
};
