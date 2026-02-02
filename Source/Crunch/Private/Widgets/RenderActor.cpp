// Fill out your copyright notice in the Description page of Project Settings.


#include "RenderActor.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"

ARenderActor::ARenderActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComp=CreateDefaultSubobject<USceneComponent>("Root Comp");
	SetRootComponent(RootComp);
	
	CaptureComponent=CreateDefaultSubobject<USceneCaptureComponent2D>("Capture Component");
	CaptureComponent->SetupAttachment(RootComp);

	//每帧渲染关闭以节省性能
	CaptureComponent->bCaptureEveryFrame = false;

	//视角缩小到30度
	CaptureComponent->FOVAngle=30.f;
}

void ARenderActor::SetRenderTarget(UTextureRenderTarget2D* RenderTarget)
{
	//将捕捉对象按照指定的Capture配置写入到RenderTarget,这里将Widget作为对象，即制作成一个Widget
	CaptureComponent->TextureTarget=RenderTarget;
}

void ARenderActor::UpdateRender()
{
	if (CaptureComponent)
	{
		//捕获指定的TextureTarget
		CaptureComponent->CaptureScene();
	}
}

void ARenderActor::BeginPlay()
{
	Super::BeginPlay();
	CaptureComponent->ShowOnlyActorComponents(this);

	//将RenderActor拉高不影响到地图内容
	SetActorLocation(FVector{0.f,0.f,100000.f});
}