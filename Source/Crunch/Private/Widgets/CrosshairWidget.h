#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "CrosshairWidget.generated.h"

struct FGameplayEventData;
struct FGameplayTag;
/**
 * 
 */
UCLASS()
class CRUNCH_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	UPROPERTY(EditDefaultsOnly,Category="View")
	FLinearColor HasTargetColor=FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly,Category="View")
	FLinearColor NoTargetColor=FLinearColor::White;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* CrosshairImage;


	UPROPERTY()
	UCanvasPanelSlot* CrosshairCanvasSlot;

	UPROPERTY()
	APlayerController* CachedPlayerController;
	
	FVector TargetPoint;
	
	UPROPERTY()
	const AActor* AimTarget;
	
	//更新可见性
	void CrosshairTagUpdated(const FGameplayTag Tag,int32 NewCount);

	//更新准星位置，保持在屏幕中心
	void UpdateCrosshairPosition();

	//更新目标，改变准星颜色
	void TargetUpdated(const FGameplayEventData* EventData);
};
