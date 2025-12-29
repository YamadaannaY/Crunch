 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemWidget.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta=(BindWidget))
	class UWrapBox* ItemList;

	UPROPERTY(EditDefaultsOnly,Category="Inventory")
	TSubclassOf<UInventoryItemWidget> ItemWidgetClass;

	UPROPERTY()
	class UInventoryComponent* InventoryComponent;

	UPROPERTY()
	TArray<UInventoryItemWidget*> ItemWidgets;

	UPROPERTY()
	TMap<FInventoryItemHandle,UInventoryItemWidget*> PopulatedItemEntryWidgets;

	
	void ItemAdded(const UInventoryItem* InventoryItem);

	UInventoryItemWidget* GetNextAvailableSlot() const ;
};
