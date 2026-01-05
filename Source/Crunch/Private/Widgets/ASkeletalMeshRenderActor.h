// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RenderActor.h"
#include "ASkeletalMeshRenderActor.generated.h"

UCLASS()
class CRUNCH_API AASkeletalMeshRenderActor : public ARenderActor
{
	GENERATED_BODY()

public:
	AASkeletalMeshRenderActor();

	void ConfigureSkeletalMeshRenderActor(USkeletalMesh* MeshAsset,TSubclassOf<UAnimInstance> AnimBlueprint);

	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere,Category="SkeletalMeshRenderActor")
	USkeletalMeshComponent* MeshComp;
};