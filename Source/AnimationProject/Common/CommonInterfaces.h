// Copyright XiaWen, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimationProject/Locomotion/LocomotionDefine.h"
#include "UObject/Interface.h"
#include "CommonInterfaces.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UAnimationInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IAnimationInterface
{
	GENERATED_IINTERFACE_BODY()
	
	UFUNCTION()
	virtual void BPIJumped() {};

	UFUNCTION()
	virtual void BPISetGroundEntryState(EGroundedEntryState NewGroundEntryState) {};

	UFUNCTION()
	virtual void BPISetOverlayOcerrideState(uint8 NewOverlayOverrideState) {};
};

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UCameraInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ICameraInterface
{
	GENERATED_IINTERFACE_BODY()
	
	UFUNCTION()
	virtual void BPIGetCameraParameters(float& TPFov, float& FPFov, bool RightShoulder) {};

	UFUNCTION()
	virtual FVector BPIGetCameraTarget() {};

	UFUNCTION()
	virtual FTransform BPIGet3PPivotTarget() {};

	UFUNCTION()
	virtual void BPIGet3PTraceParams(FVector& TraceOrigin, float& TraceRadius, TEnumAsByte<ETraceTypeQuery>& TraceChannel) {};
};

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UCharacterInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ICharacterInterface
{
	GENERATED_IINTERFACE_BODY()
	
	UFUNCTION()
	virtual void BPIGetCurrentStates(
		TEnumAsByte<EMovementMode>& OutPawnMovementMode,
		EMovementState& OutMovementState,
		EMovementState& OutPrevMovementState,
		EMovementAction& OutMovementAction,
		ERotationMode& OutRotationMode,
		EGait& OutActualGait,
		EStance& OutActualStance,
		EViewMode& OutViewMode,
		EOverlayState& OutOverlayState) {};

	UFUNCTION()
	virtual void BPIGetEssentialValues(
		FVector& OutVelocity,
		FVector& OutAcceleration,
		FVector& OutMovementInput,
		bool& OutIsMoving,
		bool& OutHasMovementInput,
		float& OutSpeed,
		float& OutMovementInputAmount,
		FRotator& OutAimingRotation,
		float& OutAimYawRate) {};

	UFUNCTION()
	virtual void BPISetMovementState(EMovementState NewMovementState) {};

	UFUNCTION()
	virtual void BPISetMovementAction(EMovementAction NewMovementAction) {};

	UFUNCTION()
    virtual void BPISetRotationMode(ERotationMode NewRotationMode) {};

	UFUNCTION()
	virtual void BPISetGait(EGait NewGait) {};
	
	UFUNCTION()
	virtual void BPISetViewMode(EViewMode NewViewMode) {};

	UFUNCTION()
	virtual void BPISetOverlayState(EOverlayState NewOverlayState) {};
};

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UControllerInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IControllerInterface
{
	GENERATED_IINTERFACE_BODY()
	
	UFUNCTION()
	virtual void BPIGetDebugInfo(ACharacter* DebugFocusCharacter,
		bool& DebugView,
		bool& ShowHUD,
		bool& ShowTraces,
		bool& ShowDebugShapes,
		bool& ShowLayerColors,
		bool& Slomo,
		bool& ShowCharacterInfo) {};
};
