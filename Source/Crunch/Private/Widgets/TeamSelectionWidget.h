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

	//设置Slot的ID，主要通过遍历所有Slot并根据遍历的遍数来确定
	void SetSlotID(uint8 NewSlotID);

	//更新Slot的显示信息
	void UpdateSlotInfo(const FString& PlayerNickName);

	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta=(BindWidget))
	class UButton* SelectButton;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* InfoText;

	//当Slot被点击时的回调
	UFUNCTION()
	void SelectButtonClicked();

	uint8 SlotID;
};
