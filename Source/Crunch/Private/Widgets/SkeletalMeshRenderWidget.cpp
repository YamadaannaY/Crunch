// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletalMeshRenderWidget.h"
#include "ASkeletalMeshRenderActor.h"
#include "RenderActorTargetInterface.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/Character.h"

void USkeletalMeshRenderWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ACharacter* PlayerCharacter=GetOwningPlayerPawn<ACharacter>();

	IRenderActorTargetInterface* PlayerCharacterRenderTargetInterface=Cast<IRenderActorTargetInterface>(PlayerCharacter);
	
	if (PlayerCharacter && SkeletalMeshRenderActor)
	{
		//将GameplayWidget的拥有者的Mesh和Anim分配给RenderActor
		SkeletalMeshRenderActor->ConfigureSkeletalMeshRenderActor(PlayerCharacter->GetMesh()->GetSkeletalMeshAsset(),PlayerCharacter->GetMesh()->GetAnimClass());
		
		USceneCaptureComponent2D* SceneCapture=SkeletalMeshRenderActor->GetCaptureComponent();
		if (PlayerCharacterRenderTargetInterface && SceneCapture)
		{
			SceneCapture->SetRelativeLocation(PlayerCharacterRenderTargetInterface->GetCaptureLocalPosition());
			SceneCapture->SetRelativeRotation(PlayerCharacterRenderTargetInterface->GetCaptureLocalRotation());
		}
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


