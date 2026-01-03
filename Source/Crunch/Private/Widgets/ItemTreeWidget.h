// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TreeNodeInterface.h"
#include "Blueprint/UserWidget.h"
#include "ItemTreeWidget.generated.h"

class UCanvasPanelSlot;
/**
 * 
 */
UCLASS()
class CRUNCH_API UItemTreeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	//
	void DrawFromNode(const ITreeNodeInterface* NodeInterface);
private:
	//
	void DrawStream(bool bUpperStream,const ITreeNodeInterface* StartingNodeInterface,UUserWidget* StartingNodeWidget,UCanvasPanelSlot* StartingNodeSlot,int StartingNodeDepth,float& NextLeafXPosition,TArray<UCanvasPanelSlot*>& OutStreamSlots);

	void ClearTree();

	//创建ItemWidget控件，放入Canvas并分配初始位置
	UUserWidget* CreateWidgetForNode(const ITreeNodeInterface* Node, class UCanvasPanelSlot*& OutCanvasSlot);

	//创建样条线控件，确定样条线位置
	void CreateConnection(const UUserWidget* From,const UUserWidget* To);
private:
	UPROPERTY(meta=(BindWidget))
	class UCanvasPanel* RootPanel;

	UPROPERTY()
	const UObject* CurrentCenterItem;
	
	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D NodeSize=FVector2D{60.f};

	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D NodeGap=FVector2D{16.f,30.f};

	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FLinearColor ConnectionColor=FLinearColor{0.8f,0.8f,0.8f,1.f};

	UPROPERTY(EditDefaultsOnly,Category="Tree")
	float ConnectionThickness=3.f;

	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D SourcePortLocalPos=FVector2D{0.5f,0.9f};
	
	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D DestinationPortLocalPos=FVector2D{0.5f,0.1f};
	
	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D SourcePortDirection=FVector2D{0.f,90.f};
	
	UPROPERTY(EditDefaultsOnly,Category="Tree")
	FVector2D DestinationPortDirection=FVector2D{0.f,90.f};
};