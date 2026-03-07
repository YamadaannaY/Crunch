// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTeamLayoutWidget.generated.h"

struct FPlayerSelection;
class UPlayerTeamSlotWidget;
class UHorizontalBox;
/**
 * 
 */
UCLASS()
class CRUNCH_API UPlayerTeamLayoutWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	//默认配置十个Slot在HorizontalBox中
	virtual void NativeConstruct() override;
	
	//每次更新Selections就遍历进行UpdateSlot
	void UpdatePlayerSelection(const TArray<FPlayerSelection>& PlayerSelections);
private:
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	TSubclassOf<UPlayerTeamSlotWidget> PlayerTeamSlotWidgetClass;
	
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	float PlayerTeamWidgetSlotMargin;
	
	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* TeamOneLayoutBox;
	
	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* TeamTwoLayoutBox;
	
	UPROPERTY()
	TArray<UPlayerTeamSlotWidget*> TeamSlotWidgets;
};
