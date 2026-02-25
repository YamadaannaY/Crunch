// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeamSelectionWidget.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotClicked,uint8 /*slotid*/);

UCLASS()
class CRUNCH_API UTeamSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	FOnSlotClicked OnSlotClicked;

	void SetSlotID(uint8 NewSlotID);
	void UpdateSlotInfo(const FString& PlayerNickName);

	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta=(BindWidget))
	class UButton* SelectButton;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* InfoText;

	UFUNCTION()
	void SelectButtonClicked();

	uint8 SlotID;
};
