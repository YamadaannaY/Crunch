#include "SkinEntryWidget.h"
#include "Character/PA_SkinDefination.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void USkinEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	SkinDefinition = Cast<UPA_SkinDefination>(ListItemObject);
	if (SkinDefinition)
	{
		if (SkinIcon && SkinIcon->GetDynamicMaterial())
		{
			SkinIcon->GetDynamicMaterial()->SetTextureParameterValue(IconTextureMatParaName, SkinDefinition->LoadSkinIcon());
		}
		if (SkinNameText)
		{
			SkinNameText->SetText(FText::FromString(SkinDefinition->GetSkinDisplayName()));
		}
	}
}

void USkinEntryWidget::SetSelected(bool bIsSelected)
{
	if (SkinIcon && SkinIcon->GetDynamicMaterial())
	{
		SkinIcon->GetDynamicMaterial()->SetScalarParameterValue(SaturationMatParaName, bIsSelected ? 0.f : 1.f);
	}
}
