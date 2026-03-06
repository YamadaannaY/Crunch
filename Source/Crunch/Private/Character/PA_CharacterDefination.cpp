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
	if (CharacterIcon.IsValid())
	{
		// 1.尝试直接从内存获取（如果已经加载过，这一步极快）
		if (UTexture2D* LoadedClass = CharacterIcon.Get())
		{
			return LoadedClass;
		}
		// 2.内存中没有，才执行同步加载
		return CharacterIcon.LoadSynchronous();
	}

	return nullptr;
}

TSubclassOf<ACCharacter> UPA_CharacterDefination::LoadCharacterClass() const
{
	if (CharacterClass.IsValid())
	{
		// 1.尝试直接从内存获取（如果已经加载过，这一步极快）
		if (UClass* LoadedClass = CharacterClass.Get())
		{
			return LoadedClass;
		}

		// 2.内存中没有，才执行同步加载
		return Cast<UClass>(CharacterClass.LoadSynchronous());
	}

	return TSubclassOf<ACCharacter>();
}

TSubclassOf<UAnimInstance> UPA_CharacterDefination::LoadDisplayAnimationBP() const
{
	if (DisplayAnimBP.IsValid())
	{
		// 1.尝试直接从内存获取（如果已经加载过，这一步极快）
		if (UClass* LoadedClass = DisplayAnimBP.Get())
		{
			return LoadedClass;
		}

		// 2.内存中没有，才执行同步加载
		return Cast<UClass>(DisplayAnimBP.LoadSynchronous());
	}
	
	return TSubclassOf<UAnimInstance>();
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
	if (!LoadedCharacterClass) return nullptr;

	ACCharacter* Character = Cast<ACCharacter>(LoadedCharacterClass.GetDefaultObject());
	if (!Character) return nullptr;

	return &Character->GetAbilities();
}
