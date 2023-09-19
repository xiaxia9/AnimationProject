// Copyright 2022-2023 XiaWen. All Rights Reserved.

/*=============================================================================
	PhysicsAnimationAsset.cpp
=============================================================================*/ 

#include "PhysicsAnimationAsset.h"
#include "Animation/MirrorDataTable.h"
#include "Engine/SkinnedAsset.h"
#include "UObject/FrameworkObjectVersion.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "UObject/ReleaseObjectVersion.h"
#include "UObject/UObjectIterator.h"
#include "UObject/FortniteNCBranchObjectVersion.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PhysicsAnimationAsset)


// #define LOCTEXT_NAMESPACE "PhysicsAnimationAsset"

UPhysicsAnimationAsset::UPhysicsAnimationAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}