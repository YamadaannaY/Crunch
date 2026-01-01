// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryItemDragDropOp.generated.h"

class UItemWidget;
class UInventoryItemWidget;

/**
 * 与目标UI的OnDragDetected和NativeOnDrop联动，在Detected中创建Op对象并且填充PayLoad，在Drop中取出Payload并Cast 
 */
UCLASS()
class CRUNCH_API UInventoryItemDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()

public:
	//以传入的Widget为PayLoad，根据Class创建Widget作为DragItem，进行Icon设置、DefaultDrag分配
	void SetDraggedItem(UInventoryItemWidget* DraggedItem);

private:
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	TSubclassOf<UItemWidget> DragVisualClass;
};
