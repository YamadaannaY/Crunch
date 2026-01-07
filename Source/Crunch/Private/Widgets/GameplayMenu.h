// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GameplayMenu.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UGameplayMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	//Button->OnClicked的委托获取，外部调用绑定回调
	FOnButtonClickedEvent& GetResumeButtonClickedEventDelegate();

	void SetTitleText(const FString& NewTitle);
private:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* MenuTitle;

	UPROPERTY(meta=(BindWidget))
	UButton* ResumeButton;

	UPROPERTY(meta=(BindWidget))
	UButton* MainMenuButton;

	UPROPERTY(meta=(BindWidget))
	UButton* QuitGameButton;

	//调用QuitGame()
	UFUNCTION()
	void BackToMainMenu();

	//退出游戏
	UFUNCTION()
	void QuitGame();
};
