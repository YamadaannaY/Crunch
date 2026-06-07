#include "Player/CPlayerController.h"
#include "CPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widgets/GameplayWidget.h"
#include "net/UnrealNetwork.h"
#include "Widgets/InventoryWidget.h"

void ACPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	//获取服务端中权威的PlayerCharacter，对其进行初始化。这样做而不是在客户端进行初始化有效防止了作弊
	CPlayerCharacter=Cast<ACPlayerCharacter>(NewPawn);

	if (CPlayerCharacter)
	{
		CPlayerCharacter->ServerSideInit();
		CPlayerCharacter->SetGenericTeamId(TeamID);
	}
}

void ACPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	
	CPlayerCharacter=Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		CPlayerCharacter->ClientSideInit();
		
		//在客户端渲染
		SpawnGameplayWidget();
	}
}

void ACPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID=NewTeamID;
}

FGenericTeamId ACPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void ACPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ACPlayerController, TeamID);
}

void ACPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem=GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		InputSubsystem->RemoveMappingContext(UIInputMapping);
		InputSubsystem->AddMappingContext(UIInputMapping,1);
	}
	
	if (UEnhancedInputComponent* EnhancedInputComp=Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComp->BindAction(ShopToggleInputAction,ETriggerEvent::Triggered,this,&ThisClass::ToggleShop);
		EnhancedInputComp->BindAction(ToggleGameplayMenuAction,ETriggerEvent::Triggered,this,&ThisClass::ToggleGameplayMenu);
	}
}

void ACPlayerController::MatchFinished(AActor* ViewTarget, int WinningTeam)
{
	//在GameMode中调用，需要具有权威端权限
	if (!HasAuthority()) return ;

	//显示UI并修改Title
	Client_MatchFinished(ViewTarget,WinningTeam);
}

void ACPlayerController::Client_MatchFinished_Implementation(AActor* ViewTarget, int WinningTeam)
{
	CPlayerCharacter->DisableInput(this);
	
	//将玩家镜头过渡到一个新的ViewTarget上
	SetViewTargetWithBlend(ViewTarget,MatchFinishedViewBlendTimeDuration);
	
	FString WinLoseMsg="You Win !";
	if (GetGenericTeamId().GetId()!=WinningTeam)
	{
		WinLoseMsg="You Lose ...." ;
	}
	GameplayWidget->SetGameplayMenuTitle(WinLoseMsg);
	
	FTimerHandle ShowWinLoseStateTimerHandle;

	//在镜头过渡完毕后调用，显示GameMenu
	GetWorldTimerManager().SetTimer(ShowWinLoseStateTimerHandle,this,&ThisClass::ShowWinLoseState,MatchFinishedViewBlendTimeDuration);
}

void ACPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController()) return;

	//本地Player拥有的视口UI
	GameplayWidget=CreateWidget<UGameplayWidget>(this,GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();

		//配置GA在ListItem中的Widget
		GameplayWidget->ConfigureAbilities(CPlayerCharacter->GetAbilities());
	}
}

void ACPlayerController::ToggleShop()
{
	if (GameplayWidget)
	{
		GameplayWidget->ToggleShop();
		GameplayWidget->InventoryWidget->ClearContextMenu();
	}
}

void ACPlayerController::ToggleGameplayMenu() 
{
	if (GameplayWidget)
	{
		//绑定IA，切换GameplayMenu
		GameplayWidget->ToggleGameplayMenu();
	}
}

void ACPlayerController::ShowWinLoseState() const 
{
	if (GameplayWidget)
	{
		//在游戏输赢时直接显示Menu
		GameplayWidget->ShowGameplayMenu();
	}
}
