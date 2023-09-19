// Copyright 2022-2023 XiaWen. All Rights Reserved.

#include "PhysicsAnimationAssetFactory.h"

UPhysicsAnimationAssetFactory::UPhysicsAnimationAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UPhysicsAnimationAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPhysicsAnimationAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UPhysicsAnimationAsset>(InParent, InClass, InName, Flags);
}

bool UPhysicsAnimationAssetFactory::ShouldShowInNewMenu() const
{
	return true;
}