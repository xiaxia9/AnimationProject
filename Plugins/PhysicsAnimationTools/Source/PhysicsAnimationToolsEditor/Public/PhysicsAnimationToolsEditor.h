// Copyright 2022-2023 XiaWen. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "AssetTypeActions_PhysicsAnimationAsset.h"
#include "Modules/ModuleManager.h"
#include "IAssetTools.h"

class FPhysicsAnimationToolsEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr<FExtensibilityManager> GetPreProcessToolkitToolbarExtensibilityManager() { return PreProcessToolkit_ToolbarExtManager;}

private:
	TSharedPtr<FExtensibilityManager> PreProcessToolkit_ToolbarExtManager;

	void RegisterAssetTools();
	void RegisterMenuExtensions();
	void RegisterMotionDataAssetTypeActions(IAssetTools& AssetTools, TSharedRef<FAssetTypeActions_PhysicsAnimationAsset> TypeActions);

	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;
};
