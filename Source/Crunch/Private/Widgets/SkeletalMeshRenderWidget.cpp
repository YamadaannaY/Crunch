// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletalMeshRenderWidget.h"

#include "ASkeletalMeshRenderActor.h"
#include "GameFramework/Character.h"

void USkeletalMeshRenderWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ACharacter* PlayerCharacter=GetOwningPlayerPawn<ACharacter>();
	if (PlayerCharacter && SkeletalMeshRenderActor)
	{
		//将GameplayWidget的拥有者的Mesh和Anim分配给RenderActor
		SkeletalMeshRenderActor->ConfigureSkeletalMeshRenderActor(PlayerCharacter->GetMesh()->GetSkeletalMeshAsset(),PlayerCharacter->GetMesh()->GetAnimClass());
	}
}

void USkeletalMeshRenderWidget::SpawnRenderActor()
{
	if (!SkeletalMeshRenderActorClass) return ;

	UWorld* World = GetWorld();
	if (!World) return ;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//将BP_RenderActor传入
	SkeletalMeshRenderActor=World->SpawnActor<AASkeletalMeshRenderActor>(SkeletalMeshRenderActorClass,SpawnParams);
	
}

ARenderActor* USkeletalMeshRenderWidget::GetRenderActor() const
{
	return SkeletalMeshRenderActor;
}


