#include "Character/PA_SkinDefination.h"

FPrimaryAssetId UPA_SkinDefination::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetSkinDefinitionAssetType(), GetFName());
}

FPrimaryAssetType UPA_SkinDefination::GetSkinDefinitionAssetType()
{
	return FPrimaryAssetType("SkinDefinition");
}

UTexture2D* UPA_SkinDefination::LoadSkinIcon() const
{
	return SkinIcon.LoadSynchronous();
}

USkeletalMesh* UPA_SkinDefination::LoadSkinMesh() const
{
	return SkinMesh.LoadSynchronous();
}
