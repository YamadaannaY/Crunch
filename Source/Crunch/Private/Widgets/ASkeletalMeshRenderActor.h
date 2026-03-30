#pragma once

#include "CoreMinimal.h"
#include "RenderActor.h"
#include "ASkeletalMeshRenderActor.generated.h"

UCLASS()
class CRUNCH_API AASkeletalMeshRenderActor : public ARenderActor
{
	GENERATED_BODY()

public:
	//配置组件，设置照明Channel
	AASkeletalMeshRenderActor();

	//配置RenderActor的Mesh和Anim
	void ConfigureSkeletalMeshRenderActor(USkeletalMesh* MeshAsset,TSubclassOf<UAnimInstance> AnimBlueprint);

	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere,Category="SkeletalMeshRenderActor")
	USkeletalMeshComponent* MeshComp;
};