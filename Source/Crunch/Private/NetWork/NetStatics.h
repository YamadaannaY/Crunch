#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NetStatics.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UNetStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//固定为5V5
	static uint8 GetPlayerCountPerTeam();
};
