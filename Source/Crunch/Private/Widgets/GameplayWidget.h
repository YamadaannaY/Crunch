// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayWidget.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//为本地UIBar调用SetAndBoundToGameplayAttribute，更新Percent和Text
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	class UValueGauge* HealthBar;
	
	UPROPERTY(meta=(BindWidget))
	UValueGauge* ManaBar;

	UPROPERTY()
	class UAbilitySystemComponent* OwnerAbilitySystemComponent;
	
};
