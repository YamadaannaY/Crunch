#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CharacterEntryWidget.generated.h"

class UPA_CharacterDefination;

/**
 * 
 */

UCLASS()
class CRUNCH_API UCharacterEntryWidget : public UUserWidget , public  IUserObjectListEntry
{
	GENERATED_BODY()
public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	FORCEINLINE const UPA_CharacterDefination* GetCharacterDefinition() const {return CharacterDefinition;}

	//修改Icon的特效参数，根据bool显示是否高亮
	void SetSelected(bool bIsSelected);

private:
	UPROPERTY(meta=(BindWidget))
	class UImage* CharacterIcon;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CharacterNameText;

	UPROPERTY(EditDefaultsOnly,Category="Character")
	FName IconTextureMatParaName = "Icon";

	UPROPERTY(EditDefaultsOnly,Category="Character")
	FName SaturationMatParaName = "Saturation";

	UPROPERTY()
	UPA_CharacterDefination* CharacterDefinition;
};
