// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "RenderActorWidget.generated.h"

class ARenderActor;
class USizeBox;
/**
 * RenderActor
 */
UCLASS(Abstract)
class CRUNCH_API URenderActorWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	//设置SizeBox用于预览
	virtual void NativePreConstruct() override;

	//生成RenderActor,进行配置，开启定时器以进行UpdateRender
	virtual void NativeConstruct() override;

	//Widget销毁时处理逻辑，销毁定时器
	virtual void BeginDestroy() override;
private:
	//为RenderActor生成一个RenderTarget，配置其尺寸和渲染形式。
	void ConfigureRenderActor();

	//在非接口中实现纯虚函数
	virtual void SpawnRenderActor() PURE_VIRTUAL(URenderActorWidget::SpawnRenderActor);
	virtual ARenderActor* GetRenderActor() const  PURE_VIRTUAL(URenderActorWidget::GetRenderActor,return nullptr;)

	void BeginRenderCapture();
	void UpdateRender();
	void StopRenderCapture();
	
	UPROPERTY(meta=(BindWidget))
	UImage* DisplayImage;

	UPROPERTY(meta=(BindWidget))
	USizeBox* RenderSizeBox;

	UPROPERTY(EditDefaultsOnly,Category="Render Actor")
	FName DisplayImageRenderTargetParaName = "RenderTarget";

	UPROPERTY(EditDefaultsOnly,Category="Render Actor")
	FVector2D RenderSize;

	UPROPERTY(EditDefaultsOnly,Category="Render Actor")
	int FrameRate=24;

	float RenderTickInterval;
	FTimerHandle RenderTimerHandle;
	
	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;
};
