#include "Character/PA_CharacterDefination.h"
#include "Character/CCharacter.h"
#include "Character/PA_SkinDefination.h"

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
	const TSubclassOf<ACCharacter> LoadedCharacterClass = LoadCharacterClass();
	if (!LoadedCharacterClass) return nullptr;

	const ACharacter* Character = Cast<ACharacter>(LoadedCharacterClass.GetDefaultObject());
	if (!Character) return nullptr;

	return Character->GetMesh()->GetSkeletalMeshAsset();
}

USkeletalMesh* UPA_CharacterDefination::LoadDisplayMeshForSkin(const UPA_SkinDefination* Skin) const
{
	// 有皮肤则用皮肤 Mesh，否则回退到默认 Mesh
	if (Skin)
	{
		USkeletalMesh* SkinMesh = Skin->LoadSkinMesh();
		if (SkinMesh)
		{
			return SkinMesh;
		}
	}
	return LoadDisplayMesh();
}

void UPA_CharacterDefination::ApplySkinToClassDefault(const UPA_SkinDefination* Skin) const
{
	if (!Skin) return;

	USkeletalMesh* SkinMesh = Skin->LoadSkinMesh();
	if (!SkinMesh) return;

	TSubclassOf<ACCharacter> LoadedClass = LoadCharacterClass();
	if (!LoadedClass) return;

	ACCharacter* CDO = Cast<ACCharacter>(LoadedClass.GetDefaultObject());
	if (CDO)
	{
		if (!CachedDefaultMesh)
		{
			CachedDefaultMesh = CDO->GetMesh()->GetSkeletalMeshAsset();
		}
		CDO->GetMesh()->SetSkeletalMesh(SkinMesh);
	}
}

void UPA_CharacterDefination::RestoreDefaultMesh() const
{
	if (!CachedDefaultMesh) return;

	TSubclassOf<ACCharacter> LoadedClass = LoadCharacterClass();
	if (!LoadedClass) return;

	ACCharacter* CDO = Cast<ACCharacter>(LoadedClass.GetDefaultObject());
	if (CDO)
	{
		CDO->GetMesh()->SetSkeletalMesh(CachedDefaultMesh);
	}
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
