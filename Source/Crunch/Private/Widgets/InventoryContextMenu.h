#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "InventoryContextMenu.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UInventoryContextMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	//封装SellButton的点击回调
	FOnButtonClickedEvent& GetSellButtonClickedEvent() const ;
	
	//封装UseButton的点击回调
	FOnButtonClickedEvent& GetUseButtonClickedEvent() const ;
private:
	UPROPERTY(meta=(BindWidget))
	UButton* SellButton;
	
	UPROPERTY(meta=(BindWidget))
	UButton* UseButton;
};
