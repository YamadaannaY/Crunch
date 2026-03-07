#include "PlayerTeamSlotWidget.h"
#include "Character/PA_CharacterDefination.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UPlayerTeamSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	NameText->SetText(FText::FromString(CacheCharacterNameStr));
	PlayAnimationForward(HoverAnim);
}

void UPlayerTeamSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	NameText->SetText(FText::FromString(CachePlayerNameStr));
	PlayAnimationReverse(HoverAnim);
}

void UPlayerTeamSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayerCharacterIcon->GetDynamicMaterial()->SetScalarParameterValue
		(CharacterEmptyMatParamName,1);
		
	CacheCharacterNameStr = "";
	
}

void UPlayerTeamSlotWidget::UpdateSlot(const FString& PlayerName, const UPA_CharacterDefination* CharacterDefinition)
{
	CachePlayerNameStr = PlayerName;
	
	if (CharacterDefinition)
	{
		PlayerCharacterIcon->GetDynamicMaterial()->SetTextureParameterValue
		(CharacterIconMatParamName,CharacterDefinition->LoadIcon());
		
		PlayerCharacterIcon->GetDynamicMaterial()->SetScalarParameterValue
		(CharacterEmptyMatParamName,0);
		
		CacheCharacterNameStr = CharacterDefinition->GetCharacterDisplayName();
	}
	else
	{
		PlayerCharacterIcon->GetDynamicMaterial()->SetScalarParameterValue
		(CharacterEmptyMatParamName,1);
		
		CacheCharacterNameStr = "";
	}
	
	UpdateNameText();
}

void UPlayerTeamSlotWidget::UpdateNameText()
{
	if (IsHovered())
	{
		NameText->SetText(FText::FromString(CacheCharacterNameStr));
	}
	else
	{
		NameText->SetText(FText::FromString(CachePlayerNameStr));
	}
}
