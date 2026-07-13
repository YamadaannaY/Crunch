#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_SkinDefination.generated.h"

/**
 * 皮肤DataAsset —— 同一个 Skeleton 的不同 Mesh 外观
 * 在编辑器中为每个英雄创建对应的皮肤资产，配置到 CharacterDefination 的 AvailableSkins 数组中
 */
UCLASS(MinimalAPI)
class UPA_SkinDefination : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	static FPrimaryAssetType GetSkinDefinitionAssetType();

	FORCEINLINE FString GetSkinDisplayName() const { return SkinName; }

	UTexture2D* LoadSkinIcon() const;
	USkeletalMesh* LoadSkinMesh() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Skin")
	FString SkinName;

	UPROPERTY(EditDefaultsOnly, Category = "Skin")
	TSoftObjectPtr<UTexture2D> SkinIcon;

	/** 皮肤 Mesh —— 必须与角色默认 Skeleton 兼容 */
	UPROPERTY(EditDefaultsOnly, Category = "Skin")
	TSoftObjectPtr<USkeletalMesh> SkinMesh;
};
