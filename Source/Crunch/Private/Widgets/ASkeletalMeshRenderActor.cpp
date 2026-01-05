// Fill out your copyright notice in the Description page of Project Settings.


#include "ASkeletalMeshRenderActor.h"


// Sets default values
AASkeletalMeshRenderActor::AASkeletalMeshRenderActor()
{
	MeshComp=CreateDefaultSubobject<USkeletalMeshComponent>("MeshComponent");
	MeshComp->SetupAttachment(GetRootComponent());
	MeshComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	//只被LightChannel为Channel1的灯光照亮
	MeshComp->SetLightingChannels(false,true,false);
}

void AASkeletalMeshRenderActor::ConfigureSkeletalMeshRenderActor(USkeletalMesh* MeshAsset,
	TSubclassOf<UAnimInstance> AnimBlueprint)
{
	//设置外观和动作逻辑
	MeshComp->SetSkeletalMeshAsset(MeshAsset);
	MeshComp->SetAnimInstanceClass(AnimBlueprint);
}

void AASkeletalMeshRenderActor::BeginPlay()
{
	Super::BeginPlay();

	//只能被捕获组件看见
	MeshComp->SetVisibleInSceneCaptureOnly(true);	
}


