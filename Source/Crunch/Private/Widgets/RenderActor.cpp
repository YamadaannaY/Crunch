// Fill out your copyright notice in the Description page of Project Settings.


#include "RenderActor.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"

// Sets default values
ARenderActor::ARenderActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComp=CreateDefaultSubobject<USceneComponent>("Root Comp");
	SetRootComponent(RootComp);
	
	CaptureComponent=CreateDefaultSubobject<USceneCaptureComponent2D>("Capture Component");
	CaptureComponent->SetupAttachment(RootComp);

	//每帧渲染关闭
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
		//捕捉触发函数，捕获到指定的TextureTarget
		CaptureComponent->CaptureScene();
	}
}

// Called when the game starts or when spawned
void ARenderActor::BeginPlay()
{
	Super::BeginPlay();
	CaptureComponent->ShowOnlyActorComponents(this);
}
