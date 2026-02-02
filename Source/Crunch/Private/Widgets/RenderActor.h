#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RenderActor.generated.h"

UCLASS()
class ARenderActor : public AActor
{
	GENERATED_BODY()

public:
	ARenderActor();

protected:
	virtual void BeginPlay() override;

public:
	//选择要捕获的对象
	void SetRenderTarget(UTextureRenderTarget2D* RenderTarget);

	//更新RenderTarget，将当前Target的改变或动画进行渲染
	void UpdateRender();

	FORCEINLINE	USceneCaptureComponent2D* GetCaptureComponent() const {return CaptureComponent;}
private:
	UPROPERTY(VisibleDefaultsOnly,Category="Render Actor")
	USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly,Category="Render Actor")
	USceneCaptureComponent2D* CaptureComponent;
};