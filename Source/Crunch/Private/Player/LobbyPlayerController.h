//在LobbyMap中配置了LobbyPlayerController用于处理Lobby的UI交互

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

	ALobbyPlayerController();
	
	//客户端UI将被点击Slot的ID发送给GameState
	UFUNCTION(Server ,Reliable , WithValidation)
	void Server_RequestSlotSelectionChange(uint8 NewSlotID);

	//服务端遍历所有PlayerController调用进入Hero选择界面的函数
	UFUNCTION(Server,Reliable,WithValidation)
	void Server_StartHeroSelection();

	//在客户端广播委托，触发绑定的回调函数，主要是通过Switcher进行Widget切换
	UFUNCTION(Client,Reliable)
	void ClientStartHeroSelection();
	
	//Button点击的回调，服务端启动MapTravel加载
	UFUNCTION(Server,Reliable , WithValidation)
	void Server_RequestionStartMatch();
};
