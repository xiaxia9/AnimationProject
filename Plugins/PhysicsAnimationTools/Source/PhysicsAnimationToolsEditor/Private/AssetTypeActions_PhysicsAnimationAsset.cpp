// Copyright 2022-2023 XiaWen. All Rights Reserved.

#include "AssetTypeActions_PhysicsAnimationAsset.h"
#include "PhysicsAnimationAsset.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_PhysicsAnimationAsset::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_PhysicsAnimationAsset", "Physics Animation Asset");
}

FColor FAssetTypeActions_PhysicsAnimationAsset::GetTypeColor() const
{
	return FColor::Blue;
}

UClass * FAssetTypeActions_PhysicsAnimationAsset::GetSupportedClass() const
{
	return UPhysicsAnimationAsset::StaticClass();
}

void FAssetTypeActions_PhysicsAnimationAsset::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		? EToolkitMode::WorldCentric
		: EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto PreProcessAsset = Cast<UPhysicsAnimationAsset>(*ObjIt);
	}
}

uint32 FAssetTypeActions_PhysicsAnimationAsset::GetCategories()
{
	return EAssetTypeCategories::Animation;
}

void FAssetTypeActions_PhysicsAnimationAsset::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder & MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
	
}

bool FAssetTypeActions_PhysicsAnimationAsset::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

bool FAssetTypeActions_PhysicsAnimationAsset::CanFilter()
{
	return true;
}

#undef LOCTEXT_NAMESPACE