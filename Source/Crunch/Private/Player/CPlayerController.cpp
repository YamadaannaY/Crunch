
#include "Player/CPlayerController.h"
#include "CPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widgets/GameplayWidget.h"
#include "net/UnrealNetwork.h"

void ACPlayerController::OnPossess(APawn* NewPawn)
{
	//这里判断了当前是服务端的PlayerController
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
	//这里判断了是在客户端的PlayerController
	Super::AcknowledgePossession(NewPawn);
	
	CPlayerCharacter=Cast<ACPlayerCharacter>(NewPawn);
	if (CPlayerCharacter)
	{
		CPlayerCharacter->ClientSideInit();
		
		//在客户端渲染。也因此Widget相关所有函数都是只在客户端执行的
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
	
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem=GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (InputSubsystem)
	{
		InputSubsystem->RemoveMappingContext(UIInputMapping);
		InputSubsystem->AddMappingContext(UIInputMapping,1);
	}
	
	UEnhancedInputComponent* EnhancedInputComp=Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(ShopToggleInputAction,ETriggerEvent::Triggered,this,&ThisClass::ToggleShop);
		EnhancedInputComp->BindAction(ToggleGameplayMenuAction,ETriggerEvent::Triggered,this,&ThisClass::ToggleGameplayMenu);
	}
}

void ACPlayerController::MatchFinished(AActor* ViewTarget, int WinningTeam)
{
	//在GameMode中调用，因此需要具有权威端
	if (!HasAuthority()) return ;

	//服务端禁用输入
	CPlayerCharacter->DisableInput(this);

	//显示UI并修改Title
	Client_MatchFinished(ViewTarget,WinningTeam);
}

void ACPlayerController::Client_MatchFinished_Implementation(AActor* ViewTarget, int WinningTeam)
{
	//将玩家镜头过渡到另一个Actor的View上
	SetViewTargetWithBlend(ViewTarget,MatchFinishedViewBlendTimeDuration);
	
	FString WinLoseMsg="You Win !";
	if (GetGenericTeamId().GetId()!=WinningTeam)
	{
		WinLoseMsg="You Lose ...." ;
	}
	
	GameplayWidget->SetGameplayMenuTitle(WinLoseMsg);
	FTimerHandle ShowWinLoseStateTimerHandle;

	//在镜头过渡完毕后调用，显示Menu
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

void ACPlayerController::ShowWinLoseState()
{
	if (GameplayWidget)
	{
		//直接显示Menu
		GameplayWidget->ShowGameplayMenu();
	}
}
