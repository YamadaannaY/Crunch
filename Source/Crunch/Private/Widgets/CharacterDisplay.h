#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "CharacterDisplay.generated.h"

class UPA_CharacterDefination;

UCLASS()
class CRUNCH_API ACharacterDisplay : public AActor
{
	GENERATED_BODY()

public:
	ACharacterDisplay();

	//将Def中的数据配置到此Actor中
	void ConfigureWithCharacterDefinition(const UPA_CharacterDefination* CharacterDefinition);

private:
	UPROPERTY(VisibleDefaultsOnly,Category="Character Display")
	USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY(VisibleDefaultsOnly,Category="Character Display")
	UCameraComponent* ViewCameraComp;
};