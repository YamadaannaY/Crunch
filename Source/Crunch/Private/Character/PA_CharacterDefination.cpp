#include "Character/PA_CharacterDefination.h"
#include "Character/CCharacter.h"

FPrimaryAssetId UPA_CharacterDefination::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetCharacterDefinitionAssetType() , GetFName());
}

FPrimaryAssetType UPA_CharacterDefination::GetCharacterDefinitionAssetType()
{
	return FPrimaryAssetType("CharacterDefinition");
}

UTexture2D* UPA_CharacterDefination::LoadIcon() const
{
	 UTexture2D* Icon=CharacterIcon.LoadSynchronous();
	if (!Icon) return nullptr;
	
	if (CharacterIcon.IsValid())
	{
		return CharacterIcon.Get();
	}

	return nullptr;
}

TSubclassOf<ACCharacter> UPA_CharacterDefination::GetCharacterClass() const
{
	TSubclassOf<ACharacter> Character = CharacterClass.LoadSynchronous();
	if (!Character) return nullptr;
	
	if (CharacterClass.IsValid()) return CharacterClass.Get();

	return TSubclassOf<ACCharacter>();
}

TSubclassOf<UAnimInstance> UPA_CharacterDefination::GetDisplayAnimationBP() const
{
	TSubclassOf<UAnimInstance> DisplayAnim = DisplayAnimBP.LoadSynchronous();
	if (!DisplayAnim) return nullptr;
	
	if (DisplayAnimBP.IsValid())
	{
		return DisplayAnimBP.Get();
	}

	return TSubclassOf<UAnimInstance>();
}

USkeletalMesh* UPA_CharacterDefination::LoadDisplayMesh() const
{
	TSubclassOf<ACCharacter> LoadedCharacterClass = GetCharacterClass();
	if (!LoadedCharacterClass) return nullptr;

	ACharacter* Character = Cast<ACharacter>(LoadedCharacterClass.GetDefaultObject());
	if (!Character) return nullptr;

	return Character->GetMesh()->GetSkeletalMeshAsset();
}