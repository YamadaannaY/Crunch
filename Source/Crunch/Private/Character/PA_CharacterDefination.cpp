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
	return CharacterIcon.LoadSynchronous();
}

TSubclassOf<ACCharacter> UPA_CharacterDefination::LoadCharacterClass() const
{
	return CharacterClass.LoadSynchronous();
}

TSubclassOf<UAnimInstance> UPA_CharacterDefination::LoadDisplayAnimationBP() const
{
	return DisplayAnimBP.LoadSynchronous();
}

USkeletalMesh* UPA_CharacterDefination::LoadDisplayMesh() const
{
	TSubclassOf<ACCharacter> LoadedCharacterClass = LoadCharacterClass();
	if (!LoadedCharacterClass) return nullptr;

	ACharacter* Character = Cast<ACharacter>(LoadedCharacterClass.GetDefaultObject());
	if (!Character) return nullptr;

	return Character->GetMesh()->GetSkeletalMeshAsset();
}

const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>* UPA_CharacterDefination::GetAbilities() const
{
	TSubclassOf<ACCharacter> LoadedCharacterClass = LoadCharacterClass();
	if (!LoadedCharacterClass)
		return nullptr;

	ACCharacter* Character = Cast<ACCharacter>(LoadedCharacterClass.GetDefaultObject());
	if (!Character)
		return nullptr;

	return &Character->GetAbilities();
}
