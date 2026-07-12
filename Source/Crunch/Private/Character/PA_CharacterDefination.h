#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_CharacterDefination.generated.h"

class UGameplayAbility;
enum class ECAbilityInputID : uint8;
class ACCharacter;
class UPA_SkinDefination;
/**
 *
 */
UCLASS(MinimalAPI)
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

	//获取指定皮肤对应的Mesh（如果Skin为nullptr则返回默认Mesh）
	USkeletalMesh* LoadDisplayMeshForSkin(const UPA_SkinDefination* Skin) const;

	//从加载好的Class中获取CDO并直接加载Abilities
	const TMap<ECAbilityInputID , TSubclassOf<UGameplayAbility>>* GetAbilities() const ;

	//获取此角色可用的所有皮肤列表
	FORCEINLINE const TArray<TSoftObjectPtr<UPA_SkinDefination>>& GetAvailableSkins() const { return AvailableSkins; }

	//将指定皮肤的 Mesh 直接设置到 CharacterClass 的 CDO 上（在 SpawnActor 前调用）
	void ApplySkinToClassDefault(const UPA_SkinDefination* Skin) const;
	
private:
	UPROPERTY(EditDefaultsOnly,Category="Character")
	FString CharacterName;

	UPROPERTY(EditDefaultsOnly,Category="Character")
	TSoftObjectPtr<UTexture2D> CharacterIcon;

	UPROPERTY(EditDefaultsOnly,Category="Character")
	TSoftClassPtr<ACharacter> CharacterClass;

	UPROPERTY(EditDefaultsOnly,Category="Character")
	TSoftClassPtr<UAnimInstance> DisplayAnimBP;

	/** 此英雄可用的所有皮肤（在编辑器中配置）。第一个元素为默认皮肤 */
	UPROPERTY(EditDefaultsOnly, Category = "Character|Skin")
	TArray<TSoftObjectPtr<UPA_SkinDefination>> AvailableSkins;
};
