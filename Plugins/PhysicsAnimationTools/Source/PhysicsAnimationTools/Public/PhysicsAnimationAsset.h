// Copyright 2022-2023 XiaWen. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "PhysicsEngine/RigidBodyIndexPair.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "Interfaces/Interface_PreviewMeshProvider.h"
#include "PhysicsAnimationAsset.generated.h"

class FMeshElementCollector;
class USkeletalBodySetup;

/**
 * PhysicsAsset contains a set of rigid bodies and constraints that make up a single ragdoll.
 * The asset is not limited to human ragdolls, and can be used for any physical simulation using bodies and constraints.
 * A SkeletalMesh has a single PhysicsAsset, which allows for easily turning ragdoll physics on or off for many SkeletalMeshComponents
 * The asset can be configured inside the Physics Asset Editor.
 *
 * @see https://docs.unrealengine.com/InteractiveExperiences/Physics/PhysicsAssetEditor
 * @see USkeletalMesh
 */

UCLASS(hidecategories=Object, BlueprintType, MinimalAPI, Config=Game, PerObjectConfig, AutoCollapseCategories=(OldSolverSettings))
// class UPhysicsAnimationAsset : public UObject, public IInterface_PreviewMeshProvider
class UPhysicsAnimationAsset : public UObject
{
	GENERATED_UCLASS_BODY()
};