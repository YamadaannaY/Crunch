// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RenderActor.generated.h"

UCLASS()
class CRUNCH_API ARenderActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARenderActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	//选择要捕获的对象
	void SetRenderTarget(UTextureRenderTarget2D* RenderTarget);

	//更新RenderTarget，将当前Target的改变或动画进行渲染
	void UpdateRender();
private:
	UPROPERTY(VisibleDefaultsOnly,Category="Render Actor")
	USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly,Category="Render Actor")
	USceneCaptureComponent2D* CaptureComponent;
};
