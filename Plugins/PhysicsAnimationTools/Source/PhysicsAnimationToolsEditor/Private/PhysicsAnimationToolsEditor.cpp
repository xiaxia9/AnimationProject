// Copyright 2022-2023 XiaWen. All Rights Reserved.

#include "PhysicsAnimationToolsEditor.h"

#include "AssetToolsModule.h"
#include "Templates/SharedPointer.h"
#include "PhysicsAnimationToolsStyle.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"

#define LOCTEXT_NAMESPACE "FPhysicsAnimationToolsEditorModule"

void FPhysicsAnimationToolsEditorModule::StartupModule()
{
	PreProcessToolkit_ToolbarExtManager = MakeShareable(new FExtensibilityManager);
	FPhysicsAnimationToolsStyle::Initialize();
	RegisterAssetTools();
	RegisterMenuExtensions();
	
}

void FPhysicsAnimationToolsEditorModule::ShutdownModule()
{
	PreProcessToolkit_ToolbarExtManager.Reset();
	FPhysicsAnimationToolsStyle::Shutdown();
}

void FPhysicsAnimationToolsEditorModule::RegisterAssetTools()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	RegisterMotionDataAssetTypeActions(AssetTools, MakeShareable(new FAssetTypeActions_PhysicsAnimationAsset()));
}

void FPhysicsAnimationToolsEditorModule::RegisterMenuExtensions()
{
	
}

void FPhysicsAnimationToolsEditorModule::RegisterMotionDataAssetTypeActions(IAssetTools& AssetTools, TSharedRef<FAssetTypeActions_PhysicsAnimationAsset> TypeActions)
{
	AssetTools.RegisterAssetTypeActions(TypeActions);
	RegisteredAssetTypeActions.Add(TypeActions);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPhysicsAnimationToolsEditorModule, PhysicsAnimationToolsEditor)