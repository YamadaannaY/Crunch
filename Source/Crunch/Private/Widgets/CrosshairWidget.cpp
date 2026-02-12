// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Crunch/DebugHelper.h"
#include "GAS/UCAbilitySystemStatics.h"

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CrosshairImage->SetVisibility(ESlateVisibility::Hidden);

	UAbilitySystemComponent* OwnerASC=UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	CachedPlayerController=GetOwningPlayer();
	
	if (OwnerASC)
	{
		OwnerASC->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetCrosshairTag()).AddUObject(this,&ThisClass::CrosshairTagUpdated);
		OwnerASC->GenericGameplayEventCallbacks.Add(UCAbilitySystemStatics::GetTargetUpdatedTag()).AddUObject(this,&ThisClass::TargetUpdated);
	}

	CrosshairCanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CrosshairCanvasSlot)
	{
		Debug::Print("crosshair widget need to be parented under a canvas panel to place itself properly");
	}

	
}

void UCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (CrosshairImage->GetVisibility()==ESlateVisibility::Visible)
	{
		UpdateCrosshairPosition();
	}
}

void UCrosshairWidget::CrosshairTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	CrosshairImage->SetVisibility(NewCount >0 ? ESlateVisibility::Visible: ESlateVisibility::Hidden);
}

void UCrosshairWidget::UpdateCrosshairPosition()
{
	if (!CachedPlayerController || !CrosshairCanvasSlot) return ;

	//DPI缩放比例，因为UI会根据分辨率不同而产生Scale作为放缩倍率，需要在计算位置的时候消除这个Scale
	const float ViewportScale=UWidgetLayoutLibrary::GetViewportScale(this);
	//获取Size
	int32 SizeX , SizeY;
	CachedPlayerController->GetViewportSize(SizeX,SizeY);

	if (!AimTarget)
	{
		const FVector2D ViewportSize=FVector2D{(float)SizeX,(float)SizeY};

		//除以2获取中心
		CrosshairCanvasSlot->SetPosition(ViewportSize/2.f/ViewportScale);
		return ;
	}

	FVector2D TargetScreenPosition;
	CachedPlayerController->ProjectWorldLocationToScreen(AimTarget->GetActorLocation(),TargetScreenPosition);
	if (TargetScreenPosition.X>0 && TargetScreenPosition.X<SizeX && TargetScreenPosition.Y>0 && TargetScreenPosition.Y<SizeY)
	{
		CrosshairCanvasSlot->SetPosition(TargetScreenPosition/ViewportScale);
	}
}

void UCrosshairWidget::TargetUpdated(const FGameplayEventData* EventData)
{
	AimTarget=EventData->Target;

	CrosshairImage->SetColorAndOpacity(AimTarget ? HasTargetColor : NoTargetColor);
}
