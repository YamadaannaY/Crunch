#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_CharacterDefination.generated.h"

class UGameplayAbility;
enum class ECAbilityInputID : uint8;
class ACCharacter;
/**
 * 
 */
UCLASS()
class UPA_CharacterDefination : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	//获取唯一的AssetID
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	//获取当前AssetType，用于加载
	static FPrimaryAssetType GetCharacterDefinitionAssetType();
	
	//获取展示Character的PlayerName
	FORCEINLINE FString GetCharacterDisplayName() const { return CharacterName; }

	//加载CharacterClass软指针
	TSubclassOf<ACCharacter> LoadCharacterClass() const ;

	//加载AnimInstance软指针
	TSubclassOf<UAnimInstance> LoadDisplayAnimationBP() const ;

	//加载Texture2D软指针
	UTexture2D* LoadIcon() const ;

	//从加载好的Class中获取CDO并加载其Mesh
	USkeletalMesh* LoadDisplayMesh() const ;

	//从加载好的Class中获取CDO并直接加载Abilities
	const TMap<ECAbilityInputID , TSubclassOf<UGameplayAbility>>* GetAbilities() const ;
private:
	UPROPERTY(EditDefaultsOnly,Category="Character")
	FString CharacterName;

	UPROPERTY(EditDefaultsOnly,Category="Character")
	TSoftObjectPtr<UTexture2D> CharacterIcon;

	UPROPERTY(EditDefaultsOnly,Category="Character")
	TSoftClassPtr<ACharacter> CharacterClass;

	UPROPERTY(EditDefaultsOnly,Category="Character")
	TSoftClassPtr<UAnimInstance> DisplayAnimBP;
};
