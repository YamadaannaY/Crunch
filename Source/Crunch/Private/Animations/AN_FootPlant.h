// AnimNotify placed at foot plant sync markers in locomotion animations.
// When consumed by SandboxAnimInstance, drives the Run→Stop transition
// at the exact frame of the next foot plant after input release.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_FootPlant.generated.h"

UENUM(BlueprintType)
enum class EFootPlant : uint8
{
	Left  UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UCLASS()
class UAN_FootPlant : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	UPROPERTY(EditAnywhere, Category = "Foot Plant")
	EFootPlant Foot = EFootPlant::Left;
};
