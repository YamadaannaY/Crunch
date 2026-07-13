#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "SkinEntryWidget.generated.h"

class UPA_SkinDefination;

/**
 * 皮肤列表中的单个条目 Widget
 * 在 WBP_SkinEntry 蓝图中继承并绑定 SkinIcon 和 SkinNameText
 */
UCLASS()
class CRUNCH_API USkinEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	FORCEINLINE const UPA_SkinDefination* GetSkinDefinition() const { return SkinDefinition; }

	// 修改选中高亮状态
	void SetSelected(bool bIsSelected);

private:
	UPROPERTY(meta = (BindWidget))
	class UImage* SkinIcon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SkinNameText;

	UPROPERTY(EditDefaultsOnly, Category = "Skin")
	FName IconTextureMatParaName = "Icon";

	UPROPERTY(EditDefaultsOnly, Category = "Skin")
	FName SaturationMatParaName = "Saturation";

	UPROPERTY()
	UPA_SkinDefination* SkinDefinition;
};
