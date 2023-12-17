// Copyright XiaWen, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveVector.h"
#include "LocomotionDefine.generated.h"

USTRUCT(BlueprintType)
struct FMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float WalkSpeed;

	UPROPERTY(BlueprintReadWrite)
	float RunSpeed;

	UPROPERTY(BlueprintReadWrite)
	float SprintSpeed;

	UPROPERTY(BlueprintReadWrite)
	UCurveVector* MovementCurve;

	UPROPERTY(BlueprintReadWrite)
	UCurveFloat* RotationRateCurve;
};

USTRUCT(BlueprintType)
struct FMovementSettingsStance
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FMovementSettings Standing;

	UPROPERTY(BlueprintReadWrite)
	FMovementSettings Crouching;
};

USTRUCT(BlueprintType)
struct FMovementSettingsState
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FMovementSettingsStance VelocityDirection;

	UPROPERTY(BlueprintReadWrite)
	FMovementSettingsStance LookingDirection;

	UPROPERTY(BlueprintReadWrite)
	FMovementSettingsStance Aiming;
};

USTRUCT(BlueprintType)
struct FVelocityBlend
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	float F = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float B = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float L = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float R = 0.0f;
};

USTRUCT(BlueprintType)
struct FMantleTraceSettings
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	float MaxLedgeHeight = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float MinLedgeHeight = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float ReachDistance = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float ForwardTraceRadius = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float DownwardTraceRadius = 0.0f;
};

USTRUCT(BlueprintType)
struct FComponentAndTransform
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FTransform Transform;

	UPROPERTY(BlueprintReadWrite)
	UPrimitiveComponent* Component = nullptr;
};

USTRUCT(BlueprintType)
struct FCameraSettings
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	float TargetArmLength = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float LagSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float RotationLagSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	bool DoCollisionTest = true;
};

USTRUCT(BlueprintType)
struct FCameraSettingsGait
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FCameraSettings Walking;

	UPROPERTY(BlueprintReadWrite)
	FCameraSettings Running;

	UPROPERTY(BlueprintReadWrite)
	FCameraSettings Sprinting;

	UPROPERTY(BlueprintReadWrite)
	FCameraSettings Crouching;
};

USTRUCT(BlueprintType)
struct FCameraSettingsState
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FCameraSettings VelocityDirection;

	UPROPERTY(BlueprintReadWrite)
	FCameraSettings LookingDirection;

	UPROPERTY(BlueprintReadWrite)
	FCameraSettings Aiming;
};

USTRUCT(BlueprintType)
struct FDynamicMontageParams
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float BlendInTime = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float BlendOutTime = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float PlayRate = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float StartTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FLeanAmount
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float LR = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float FB = 0.0f;
};

USTRUCT(BlueprintType)
struct FMantleAsset
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> AnimMontage = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Position/Correction Curve"))
	FVectorCurve* PositionCurve = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FVector StartingOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float LowHeight = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float LowPlayRate = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float LowStartPosition = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float HighHeight = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float HighPlayRate = 0.0f;
	
	UPROPERTY(BlueprintReadWrite)
	float HightStartPosition = 0.0f;
};

USTRUCT(BlueprintType)
struct FMantleParams
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> AnimMontage = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "Position/Correction Curve"))
	FVectorCurve* PositionCurve = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float StartingPosition = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float PlayRate = 0.0f;
	
	UPROPERTY(BlueprintReadWrite)
	FVector StartingOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float LowHeight = 0.0f;
};

USTRUCT(BlueprintType)
struct FRotateInPlaceAsset
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FName SlotName;
	
	UPROPERTY(BlueprintReadWrite)
	float SlowTurnRate = 0.0f;
	
	UPROPERTY(BlueprintReadWrite)
	float FastTurnRate = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float SlowPlayRate = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float FastPlayRate = 0.0f;
};

USTRUCT(BlueprintType)
struct FTurnInPlaceAsset
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float AnimatedAngle = 0.0f;
	
	UPROPERTY(BlueprintReadWrite)
	FName SlotName;
	
	UPROPERTY(BlueprintReadWrite)
	float PlayRate = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	bool ScaleTurnAngle = true;
};

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walking,
	Running,
	Sprinting
};

UENUM(BlueprintType)
enum class EMovementAction : uint8
{
	None,
	LowMantle,
	HighMantle,
	Rolling,
	GettingUp
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	None,
	Grounded,
	InAir,
	Mantling,
	Ragdoll
};

UENUM(BlueprintType)
enum class EOverlayState : uint8
{
	Default,
	Masculine,
	Feminine,
	Injured,
	HandsTied,
	Rifle,
	Pistol1H,
	Pistol2H,
	Bow,
	Torch,
	Binoculars,
	Box,
	Barrel
};

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	VelocityDirection,
	LookingDirection,
	Aiming
};

UENUM(BlueprintType)
enum class EStance : uint8
{
	Standing,
	Crouching
};

UENUM(BlueprintType)
enum class EViewMode : uint8
{
	ThirdPerson,
	FirstPerson
};

UENUM(BlueprintType)
enum class EAnimFeatureExample : uint8
{
	StrideBlending,
	AdditiveLeaning,
	SprintImpulse
};

UENUM(BlueprintType)
enum class EFootstepType : uint8
{
	Step,
	WalkOrRun,
	Jump,
	Land
};

UENUM(BlueprintType)
enum class EGroundedEntryState : uint8
{
	None,
	Roll
};

UENUM(BlueprintType)
enum class EHipsDirection : uint8
{
	F,
	B,
	RF,
	RB,
	LF,
	LB
};

UENUM(BlueprintType)
enum class EMantleType : uint8
{
	HighMantle,
	LowMantle,
	FallingCatch
};

UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	Forward,
	Right,
	Left,
	Backward
};
