// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "CrosshairWidget.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class CRUNCH_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* CrosshairImage;

	void CrosshairTagUpdated(const FGameplayTag Tag,int32 NewCount);

	UPROPERTY()
	UCanvasPanelSlot* CrosshairCanvasSlot;

	UPROPERTY()
	APlayerController* CachedPlayerController;

	void UpdateCrosshairPosition();
};
