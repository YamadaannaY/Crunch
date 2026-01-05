// Fill out your copyright notice in the Description page of Project Settings.


#include "RenderActorWidget.h"

#include "RenderActor.h"
#include "Components/SizeBox.h"
#include "Crunch/DebugHelper.h"
#include "Engine/TextureRenderTarget2D.h"

void URenderActorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	RenderSizeBox->SetWidthOverride(RenderSize.X);
	RenderSizeBox->SetHeightOverride(RenderSize.Y);
}

void URenderActorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SpawnRenderActor();
	ConfigureRenderActor();
	BeginRenderCapture();
}

void URenderActorWidget::BeginDestroy()
{
	StopRenderCapture();
	Super::BeginDestroy();
}

void URenderActorWidget::ConfigureRenderActor()
{
	if (!GetRenderActor())
	{
		Debug::Print("No Render Actor ! Noting will be Capture");
		return;
	}

	//将Widget作为Render对象，并设置分配率为SizeBox大小
	RenderTarget=NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->InitAutoFormat((int)RenderSize.X,(int)RenderSize.Y);

	//进行色彩校正，提供透明背景效果
	RenderTarget->RenderTargetFormat=RTF_RGBA8_SRGB;

	//为RenderActor确定Target，每次进行Capture都会将画面分配到Widget上
	GetRenderActor()->SetRenderTarget(RenderTarget);

	UMaterialInstanceDynamic* DisplayImageDynamicMaterial=DisplayImage->GetDynamicMaterial();
	if (DisplayImageDynamicMaterial)
	{
		//将DynamicIcon内部指针指向RenderTarget,实时更新其画面变化
		DisplayImageDynamicMaterial->SetTextureParameterValue(DisplayImageRenderTargetParaName,RenderTarget);
	}
}

void URenderActorWidget::BeginRenderCapture()
{
	//24帧渲染速度
	RenderTickInterval=1.f/(float)FrameRate;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(RenderTimerHandle,this,&URenderActorWidget::UpdateRender,RenderTickInterval,true);	
	}
}

void URenderActorWidget::UpdateRender()
{
	if (GetRenderActor())
	{
		GetRenderActor()->UpdateRender();
	}
}

void URenderActorWidget::StopRenderCapture()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(RenderTimerHandle);
	}
}