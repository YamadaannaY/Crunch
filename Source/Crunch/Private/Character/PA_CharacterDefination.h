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

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	static FPrimaryAssetType GetCharacterDefinitionAssetType();
	
	FString GetCharacterDisplayName() const { return CharacterName; }

	//查询/加载软指针
	TSubclassOf<ACCharacter> LoadCharacterClass() const ;

	//查询加载软指针
	TSubclassOf<UAnimInstance> LoadDisplayAnimationBP() const ;

	UTexture2D* LoadIcon() const ;

	USkeletalMesh* LoadDisplayMesh() const ;

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
