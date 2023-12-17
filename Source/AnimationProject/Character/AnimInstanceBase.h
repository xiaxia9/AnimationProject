// Copyright XiaWen, Inc. All Rights Reserved.

#pragma once

#include "CharacterBase.h"
#include "Animation/AnimInstance.h"
#include "AnimationProject/Common/CommonInterfaces.h"
#include "AnimInstanceBase.generated.h"

UCLASS(Config = Game)
class UAnimInstanceBase : public UAnimInstance, public IAnimationInterface
{
	GENERATED_BODY()

public:
	UAnimInstanceBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	virtual void BPIJumped() override;
	virtual void BPISetGroundEntryState(EGroundedEntryState NewGroundEntryState) override;
	virtual void BPISetOverlayOcerrideState(uint8 NewOverlayOverrideState) override;

protected:
	// todo event
	void PlayTransition(FDynamicMontageParams Parameters);
	void AnimNotify_StopTransition();
	void PlayDynamicTransition(float ReTriggerDelay, FDynamicMontageParams Parameters);
	void AnimNotify_NStopR();
	void AnimNotify_NStopL();
	void AnimNotify_CLFStop();
	void AnimNotify_RollIdle();
	void AnimNotify_LandIdle();
	void AnimNotify_BowRelaxedReady();
	void AnimNotify_BowReadyRelaxed();
	void AnimNotify_M4A1RelaxedReady();
	void AnimNotify_M4A1ReadyRelaxed();
	void AnimNotify_Pistol1HRelaxedReady();
	void AnimNotify_Pistol1HReadyRelaxed();
	void AnimNotify_Pistol2HRelaxedReady();
	void AnimNotify_Pistol2HReadyRelaxed();
	void AnimNotify_Pivot();
	void AnimNotify_HipsF();
	void AnimNotify_HipsB();
	void AnimNotify_HipsLF();
	void AnimNotify_HipsLB();
	void AnimNotify_HipsRF();
	void AnimNotify_HipsRB();
	void AnimNotify_HResetGroundedEntryState();
	
private:
	void UpdateCharacterInfo();
	void UpdateAimingValues();
	void UpdateLayerValues();
	void UpdateFootIK();
	void UpdateInAirValues();
	void UpdateRagdollValues();
	void UpdateMovementValues();
	void UpdateRotationValues();
	bool ShouldMoveCheck();
	void RotateInPlaceCheck();
	void TurnInPlaceCheck();
	void DynamicTransitionCheck();
	bool CanRotateInPlace();
	bool CanTurnInPlace();
	bool CanDynamicTransition();
	bool CanOverlayTransition();
	void SetFootLocking(FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone, float& CurrentFootLockAlpha,
		FVector& CurrentFootLockLocation, FRotator& CurrentFootLockRotation);
	void SetFootOffsets(FName EnableFootIKCurve, FName IKFootBone, FName RootBone, FVector& CurrentLocationTarget,
		FVector& CurrentLocationOffset, FRotator& CurrentRotationOffset);
	void SetPelvisIKOffset(FVector FootOffsetLTarget, FVector FootOffsetRTarget);
	void ResetIKOffsets();
	float CalculateLandPrediction();
	FLeanAmount InterpLeanAmount(FLeanAmount Current, FLeanAmount Target, float InterpSpeed, float DeltaTime);
	FLeanAmount CalculateInAirLeanAmount();
	FVelocityBlend InterpVelocityBlend(FVelocityBlend Current, FVelocityBlend Target, float InterpSpeed, float DeltaTime);
	FVelocityBlend CalculateVelocityBlend();
	float CalculateDiagonalScaleAmount();
	FVector CalculateRelativeAccelerationAmount();
	float CalculateWalkRunBlend();
	float CalculateStrideBlend();
	float CalculateStandingPlayRate();
	float CalculateCrouchingPlayRate();
	EMovementDirection CalculateMovementDirection();
	void TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);
	void SetFootLockOffsets(FVector& LocalLocation, FRotator& LocalRotation);
	EMovementDirection CalculateQuadrant(EMovementDirection Current, float FRThreshold, float FLThreshold, float BRThreshold, float BLThreshold, float Buffer, float Angle);
	bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer);
	
private:
	float DeltaTimeX = 0.0f;
	// weakptr
	ACharacterBase* CharacterBase;
	EMovementState MovementState = EMovementState::Grounded;
	bool bShouldMove = false;
	bool bRotateL = false;
	bool bRotateR = false;
	float RotateRate = 0.0f;
	float ElapsedDelayTime = 0.0f;
	EHipsDirection TrackedHipsDirection = EHipsDirection::F;
	float Speed = 0.0f;
	float TriggerRivotSpeedLimit = 0.0f;
	bool bPivot = false;
	bool bJumped = false;
	float JumpPlayRate = 0.0f;
	float MinPlayRate = 0.0f;
	float MaxPlayRate = 0.0f;
	EGroundedEntryState GroundEntryState = EGroundedEntryState::None;
	uint8 OverlayOverrideState = 0;
	
	FVector Velocity = FVector::ZeroVector;
	FVector Acceleration = FVector::ZeroVector;
	FVector MovementInput = FVector::ZeroVector;
	bool bIsMoving = false;
	bool bHasMovementInput = false;
	float MovementInputAmount = 0.0f;
	FRotator AimingRotation = FRotator::ZeroRotator;
	float AimYawRate = 0.0f;
	float AimYawRateMinRange = 0.0f;
	float AimYawRateMaxRange = 0.0f;
	float AimYawRateLimit = 0.0f;
	
	TEnumAsByte<EMovementMode> PawnMovementMode = EMovementMode::MOVE_Walking;
	EMovementState PrevMovementState = EMovementState::Grounded;
	EMovementAction MovementAction = EMovementAction::None;
	ERotationMode RotationMode = ERotationMode::VelocityDirection;
	EGait ActualGait = EGait::Walking;
	EGait Gait = EGait::Walking;
	EStance Stance = EStance::Standing;
	EStance ActualStance = EStance::Standing;
	EViewMode ViewMode = EViewMode::ThirdPerson;
	EOverlayState OverlayState = EOverlayState::Pistol1H;

	FRotator SmoothedAimingRotation = FRotator::ZeroRotator;
	float SmoothedAimingRotationInterpSpeed = 0.0f;
	FVector2d AimingAngle = FVector2d::ZeroVector;
	FVector2d SmoothedAimingAngle = FVector2d::ZeroVector;
	float AimSweepTime = 0.0f;
	FRotator SpineRotation = FRotator::ZeroRotator;
	float InputYawOffsetTime = 0.0f;
	float InputYawOffsetInterpSpeed = 8.0f;
	float LeftYawTime = 0.0f;
	float RightYawTime = 0.0f;
	float ForwardYawTime = 0.0f;

	float EnableAimOffset = 0.0f;
	float BasePoseN = 0.0f;
	float BasePoseCLF = 0.0f;
	float SpineAdd = 0.0f;
	float HeadAdd = 0.0f;
	float ArmLAdd = 0.0f;
	float ArmRAdd = 0.0f;
	float HandR = 0.0f;
	float HandL = 0.0f;
	float EnableHandIKL = 0.0f;
	float EnableHandIKR = 0.0f;
	float ArmLLS = 0.0f;
	float ArmRLS = 0.0f;
	float ArmLMS = 0.0f;
	float ArmRMS = 0.0f;

	float FootLockLAlpha = 0.0f;
	FVector FootLockLLocation = FVector::ZeroVector;
	FRotator FootLockLRotation = FRotator::ZeroRotator;
	float FootLockRAlpha = 0.0f;
	FVector FootLockRLocation = FVector::ZeroVector;
	FRotator FootLockRRotation = FRotator::ZeroRotator;
	FVector FootOffsetLTarget = FVector::ZeroVector;
	FVector FootOffsetLLocation = FVector::ZeroVector;
	FRotator FootOffsetLRotation = FRotator::ZeroRotator;
	FVector FootOffsetRTarget = FVector::ZeroVector;
	FVector FootOffsetRLocation = FVector::ZeroVector;
	FRotator FootOffsetRRotation = FRotator::ZeroRotator;

	float FallSpeed = 0.0f;
	float LandPrediction = 0.0f;
	FLeanAmount LeanAmount;
	float InAirLeanInterpSpeed = 0.0f;

	float FlailRate = 0.0f;

	FVelocityBlend VelocityBlend;
	float VelocityBlendInterpSpeed = 0.0f;
	float DiagonalScaleAmount = 0.0f;
	FVector RelativeAccelerationAmount = FVector::ZeroVector;
	float GroundedLeanInterpSpeed = 0.0f;
	float WalkRunBlend = 0.0f;
	float StrideBlend = 0.0f;
	float StandingPlayRate = 0.0f;
	float CrouchingPlayRate = 0.0f;

	EMovementDirection MovementDirection = EMovementDirection::Forward;
	UCurveVector* YawOffsetFB = nullptr;
	UCurveVector* YawOffsetLR = nullptr;
	float FYaw = 0.0f;
	float BYaw = 0.0f;
	float LYaw = 0.0f;
	float RYaw = 0.0f;

	float RotateMinThreshold = 0.0f;
	float RotateMaxThreshold = 0.0f;
	float TurnCheckMinAngle = 0.0f;
	float MinAngleDelay = 0.0f;
	float MaxAngleDelay = 0.0f;

	float FootLockCurveValue = 0.0f;
	float IKTraceDistanceAboveFoot = 0.0f;
	float IKTraceDistanceBelowFoot = 0.0f;
	float FootHeight = 0.0f;

	float PelvisAlpha = 0.0f;
	FVector PelvisOffset = FVector::ZeroVector;

	UCurveFloat* LeanInAirCurve = nullptr;
	UCurveFloat* DiagonalScaleAmountCurve = nullptr;
	UCurveFloat* StrideBlendNWalk = nullptr;
	UCurveFloat* StrideBlendNRun = nullptr;
	UCurveFloat* StrideBlendCWalk = nullptr;
	UCurveFloat* LandPredictionCurve = nullptr;

	float AnimatedWalkSpeed = 0.0f;
	float AnimatedRunSpeed = 0.0f;
	float AnimatedSprintSpeed = 0.0f;
	float AnimatedCrouchSpeed = 0.0f;

	float Turn180Threshold = 0.0f;
	FTurnInPlaceAsset NTurnIPL90;
	FTurnInPlaceAsset CLFTurnIPL90;
	FTurnInPlaceAsset NTurnIPR90;
	FTurnInPlaceAsset CLFTurnIPR90;
	FTurnInPlaceAsset NTurnIPL180;
	FTurnInPlaceAsset CLFTurnIPL180;
	FTurnInPlaceAsset NTurnIPR180;
	FTurnInPlaceAsset CLFTurnIPR180;
	float RotationScale = 0.0f;
};
