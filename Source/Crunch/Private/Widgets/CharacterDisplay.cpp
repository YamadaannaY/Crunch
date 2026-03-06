#include "CharacterDisplay.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/PA_CharacterDefination.h"

ACharacterDisplay::ACharacterDisplay()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Comp"));

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh Comp");
	SkeletalMeshComp->SetupAttachment(GetRootComponent());
	
	ViewCameraComp = CreateDefaultSubobject<UCameraComponent>("View Camera");
	ViewCameraComp->SetupAttachment(GetRootComponent());
}

void ACharacterDisplay::ConfigureWithCharacterDefinition(const UPA_CharacterDefination* CharacterDefinition)
{
	if (!CharacterDefinition) return ;

	SkeletalMeshComp->SetSkeletalMesh(CharacterDefinition->LoadDisplayMesh());
	SkeletalMeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	SkeletalMeshComp->SetAnimInstanceClass(CharacterDefinition->LoadDisplayAnimationBP());
}