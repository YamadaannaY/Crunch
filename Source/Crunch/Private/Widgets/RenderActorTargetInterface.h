
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RenderActorTargetInterface.generated.h"

//这个Interface被Character继承，用于在CharacterClass中确定RenderActor的位置，在TextureTarget（即Widget）中传入

UINTERFACE()
class URenderActorTargetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IRenderActorTargetInterface
{
	GENERATED_BODY()

public:
	virtual FVector GetCaptureLocalPosition() const =0 ;
	virtual FRotator GetCaptureLocalRotation() const =0;
};
