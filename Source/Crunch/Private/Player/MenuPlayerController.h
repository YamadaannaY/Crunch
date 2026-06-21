#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"

/**
 * Menu相关的PlayerController，处理纯UI交互 
 */
UCLASS()
class CRUNCH_API AMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;

private:
	UPROPERTY(EditDefaultsOnly,Category="Menu")
	TSubclassOf<UUserWidget> MenuWidgetClass;

	UPROPERTY()
	UUserWidget* MenuWidget;

	//生成MenuWidget并加入Viewport
	void SpawnWidget();
};
