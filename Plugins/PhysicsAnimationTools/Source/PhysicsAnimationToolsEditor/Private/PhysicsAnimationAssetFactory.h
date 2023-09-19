// Copyright 2022-2023 XiaWen. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PhysicsAnimationAsset.h"
#include "PhysicsAnimationAssetFactory.generated.h"

/**
 * 
 */
UCLASS(hidecategories=Object)
class UPhysicsAnimationAssetFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UFactory interface
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual bool ShouldShowInNewMenu() const override;
	//~ End UFactory Interface
};
