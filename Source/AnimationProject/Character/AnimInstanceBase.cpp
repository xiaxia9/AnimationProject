// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimInstanceBase.h"
#include "CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UAnimInstanceBase::UAnimInstanceBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (APawn* Pawn = TryGetPawnOwner())
	{
		CharacterBase = Cast<ACharacterBase>(Pawn);
	}
}

void UAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	DeltaTimeX = DeltaSeconds;
	
	if (DeltaTimeX == 0.0f || !IsValid(CharacterBase))
	{
		return;
	}

	UpdateCharacterInfo();
	UpdateAimingValues();
	UpdateLayerValues();
	UpdateFootIK();
	switch (MovementState)
	{
	case EMovementState::Grounded:
		bShouldMove = ShouldMoveCheck();
		// todo, ML DoWhile
		if (bShouldMove)
		{
			UpdateMovementValues();
			UpdateRotationValues();
		}
		else
		{
			if (CanRotateInPlace())
			{
				RotateInPlaceCheck();
			}
			else
			{
				bRotateL = false;
				bRotateR = false;
			}

			if (CanTurnInPlace())
			{
				TurnInPlaceCheck();
			}
			else
			{
				ElapsedDelayTime = 0.0f;
			}

			if (CanDynamicTransition())
			{
				DynamicTransitionCheck();
			}
		}
		// ChangedToTrue
		ElapsedDelayTime = 0.0f;
		bRotateL = false;
		bRotateR = false;
		break;
	case EMovementState::InAir:
		UpdateInAirValues();
		break;
	case EMovementState::Ragdoll:
		UpdateRagdollValues();
		break;
	default:
		break;
	}
}

void UAnimInstanceBase::BPIJumped()
{
	bJumped = true;
	JumpPlayRate = UKismetMathLibrary::MapRangeClamped(Speed, 0.0f, 600.0f, 1.2f, 1.5f);
	// todo delay 0.1
	bJumped = false;
}

void UAnimInstanceBase::BPISetGroundEntryState(EGroundedEntryState NewGroundEntryState)
{
	GroundEntryState = NewGroundEntryState;
}

void UAnimInstanceBase::BPISetOverlayOcerrideState(uint8 NewOverlayOverrideState)
{
	OverlayOverrideState = NewOverlayOverrideState;
}

void UAnimInstanceBase::AnimNotify_HResetGroundedEntryState()
{
	GroundEntryState = EGroundedEntryState::None;
}

void UAnimInstanceBase::PlayTransition(FDynamicMontageParams Parameters)
{
	PlaySlotAnimationAsDynamicMontage(Parameters.Animation, FName("Grounded Slot"), Parameters.BlendInTime,
		Parameters.BlendOutTime, Parameters.PlayRate, 1, 0.0f, Parameters.StartTime);
}

void UAnimInstanceBase::AnimNotify_StopTransition()
{
	StopSlotAnimation(0.2f, FName("Grounded Slot"));
	StopSlotAnimation(0.2f, FName("(N) Turn/Rotate"));
	StopSlotAnimation(0.2f, FName("(CLF) Turn/Rotate"));
}

void UAnimInstanceBase::PlayDynamicTransition(float ReTriggerDelay, FDynamicMontageParams Parameters)
{
	// todo delay ReTriggerDelay, use set timer
	// todo gate
	PlaySlotAnimationAsDynamicMontage(Parameters.Animation, FName("Grounded Slot"), Parameters.BlendInTime,
		Parameters.BlendOutTime, Parameters.PlayRate, 1, 0.0f, Parameters.StartTime);
}

void UAnimInstanceBase::AnimNotify_NStopR()
{
	PlayTransition(FDynamicMontageParams(nullptr, 0.2f, 0.2f, 1.5f, 0.4f));
}

void UAnimInstanceBase::AnimNotify_NStopL()
{
	PlayTransition(FDynamicMontageParams(nullptr, 0.2f, 0.2f, 1.5f, 0.4f));
}

void UAnimInstanceBase::AnimNotify_CLFStop()
{
	PlayTransition(FDynamicMontageParams(nullptr, 0.2f, 0.2f, 1.5f, 0.4f));
}

void UAnimInstanceBase::AnimNotify_RollIdle()
{
	PlayTransition(FDynamicMontageParams(nullptr, 0.2f, 0.2f, 1.5f, 0.2f));
}

void UAnimInstanceBase::AnimNotify_LandIdle()
{
	PlayTransition(FDynamicMontageParams(nullptr, 0.1f, 0.2f, 1.4f, 0.0f));
}

void UAnimInstanceBase::AnimNotify_BowRelaxedReady()
{
	if (CanOverlayTransition())
	{
		PlayTransition(FDynamicMontageParams(nullptr, 0.1f, 0.2f, 1.4f, 0.0f));
	}
}

void UAnimInstanceBase::AnimNotify_BowReadyRelaxed()
{
	if (CanOverlayTransition())
	{
		PlayTransition(FDynamicMontageParams(nullptr, 0.1f, 0.2f, 1.4f, 0.0f));
	}
}

void UAnimInstanceBase::AnimNotify_M4A1RelaxedReady()
{
	if (CanOverlayTransition())
	{
		PlayTransition(FDynamicMontageParams(nullptr, 0.1f, 0.2f, 1.4f, 0.0f));
	}
}

void UAnimInstanceBase::AnimNotify_M4A1ReadyRelaxed()
{
	if (CanOverlayTransition())
	{
		PlayTransition(FDynamicMontageParams(nullptr, 0.1f, 0.2f, 1.4f, 0.0f));
	}
}

void UAnimInstanceBase::AnimNotify_Pistol1HRelaxedReady()
{
	if (CanOverlayTransition())
	{
		PlayTransition(FDynamicMontageParams(nullptr, 0.1f, 0.2f, 1.4f, 0.0f));
	}
}

void UAnimInstanceBase::AnimNotify_Pistol1HReadyRelaxed()
{
	if (CanOverlayTransition())
	{
		PlayTransition(FDynamicMontageParams(nullptr, 0.1f, 0.2f, 1.4f, 0.0f));
	}
}

void UAnimInstanceBase::AnimNotify_Pistol2HRelaxedReady()
{
	if (CanOverlayTransition())
	{
		PlayTransition(FDynamicMontageParams(nullptr, 0.1f, 0.2f, 1.4f, 0.0f));
	}
}

void UAnimInstanceBase::AnimNotify_Pistol2HReadyRelaxed()
{
	if (CanOverlayTransition())
	{
		PlayTransition(FDynamicMontageParams(nullptr, 0.1f, 0.2f, 1.4f, 0.0f));
	}
}

void UAnimInstanceBase::AnimNotify_Pivot()
{
	bPivot = Speed < TriggerRivotSpeedLimit;
	// todo delay 0.1
	bPivot = false;
}

void UAnimInstanceBase::AnimNotify_HipsF()
{
	TrackedHipsDirection = EHipsDirection::F;
}

void UAnimInstanceBase::AnimNotify_HipsB()
{
	TrackedHipsDirection = EHipsDirection::B;
}

void UAnimInstanceBase::AnimNotify_HipsLF()
{
	TrackedHipsDirection = EHipsDirection::LF;
}

void UAnimInstanceBase::AnimNotify_HipsLB()
{
	TrackedHipsDirection = EHipsDirection::LB;
}

void UAnimInstanceBase::AnimNotify_HipsRF()
{
	TrackedHipsDirection = EHipsDirection::RF;
}

void UAnimInstanceBase::AnimNotify_HipsRB()
{
	TrackedHipsDirection = EHipsDirection::RB;
}

void UAnimInstanceBase::UpdateCharacterInfo()
{
	if (IsValid(CharacterBase))
	{
		// 使用结构体
		CharacterBase->BPIGetEssentialValues(Velocity, Acceleration, MovementInput, bIsMoving,
			bHasMovementInput, Speed, MovementInputAmount, AimingRotation, AimYawRate);

		CharacterBase->BPIGetCurrentStates(PawnMovementMode, MovementState, PrevMovementState,
			MovementAction, RotationMode, ActualGait, ActualStance, ViewMode, OverlayState);
	}
}

void UAnimInstanceBase::UpdateAimingValues()
{
	SmoothedAimingRotation = UKismetMathLibrary::RInterpTo(
		SmoothedAimingRotation, AimingRotation, DeltaTimeX, SmoothedAimingRotationInterpSpeed);
	
	FRotator DeltaRotation = AimingRotation - CharacterBase->GetActorRotation();
	AimingAngle = FVector2d(DeltaRotation.Yaw, DeltaRotation.Pitch);
	FRotator DeltaSmoothedRotation = SmoothedAimingRotation - CharacterBase->GetActorRotation();
	SmoothedAimingAngle = FVector2d(DeltaSmoothedRotation.Yaw, DeltaSmoothedRotation.Pitch);

	switch (RotationMode)
	{
	case ERotationMode::LookingDirection:
	case ERotationMode::Aiming:
		AimSweepTime = UKismetMathLibrary::MapRangeClamped(AimingAngle.Y, -90.0f, 90.0f, 1.0f, 0.0f);
		SpineRotation = FRotator(0.0f, 0.0f, AimingAngle.X / 4.0f);
		break;
	default:
		break;
	}

	switch (RotationMode)
	{
	case ERotationMode::VelocityDirection:
		if (bHasMovementInput)
		{
			FRotator DeltaRotation = MovementInput.Rotation() - CharacterBase->GetActorRotation();
			float ClampedValue = UKismetMathLibrary::MapRangeClamped(DeltaRotation.Yaw, -180.0f, 180.0f, 0.0f, 1.0f);
			InputYawOffsetTime = UKismetMathLibrary::FInterpTo(InputYawOffsetTime, ClampedValue, DeltaTimeX, InputYawOffsetInterpSpeed);
		}
		break;
	default:
		break;
	}

	LeftYawTime = UKismetMathLibrary::MapRangeClamped(FMath::Abs(SmoothedAimingAngle.X), 0.0f, 180.0f, 0.5f, 0.0);
	RightYawTime = UKismetMathLibrary::MapRangeClamped(FMath::Abs(SmoothedAimingAngle.X), 0.0f, 180.0f, 0.5f, 1.0);
	ForwardYawTime = UKismetMathLibrary::MapRangeClamped(FMath::Abs(SmoothedAimingAngle.X), -180.0f, 180.0f, 0.0f, 1.0);
}

void UAnimInstanceBase::UpdateLayerValues()
{
	EnableAimOffset = FMath::Lerp(1.0f, 0.0f, GetCurveValue(FName("Mask_AimOffset")));
	BasePoseN = GetCurveValue(FName("BasePose_N"));
	BasePoseCLF = GetCurveValue(FName("BasePose_CLF"));
	SpineAdd = GetCurveValue(FName("Layering_Spine_Add"));
	HeadAdd = GetCurveValue(FName("Layering_Head_Add"));
	ArmLAdd = GetCurveValue(FName("Layering_Arm_L_Add"));
	ArmRAdd = GetCurveValue(FName("Layering_Arm_R_Add"));
	HandR = GetCurveValue(FName("Layering_Hand_R"));
	HandL = GetCurveValue(FName("Layering_Hand_L"));
	EnableHandIKL = FMath::Lerp(0.0f, GetCurveValue(FName("Enable_HandIK_L")), GetCurveValue(FName("Layering_Arm_L")));
	EnableHandIKR = FMath::Lerp(0.0f, GetCurveValue(FName("Enable_HandIK_L")), GetCurveValue(FName("Layering_Arm_R")));
	ArmLLS = GetCurveValue(FName("Layering_Arm_L_LS"));
	ArmRLS = GetCurveValue(FName("Layering_Arm_R_LS"));
	ArmLMS = 1 - FMath::Floor(ArmLLS);
	ArmRMS = 1 - FMath::Floor(ArmRLS);
}

void UAnimInstanceBase::UpdateFootIK()
{
	SetFootLocking(FName("Enable_FootIK_L"), FName("FootLock_L"), FName("ik_foot_l"),
		FootLockLAlpha, FootLockLLocation, FootLockLRotation);
	SetFootLocking(FName("Enable_FootIK_R"), FName("FootLock_R"), FName("ik_foot_r"),
	FootLockRAlpha, FootLockRLocation, FootLockRRotation);
	switch (MovementState)
	{
	case EMovementState::None:
	case EMovementState::Grounded:
	case EMovementState::Mantling:
		SetFootOffsets(FName("Enable_FootIK_L"), FName("ik_foot_l"), FName("root"),
			FootOffsetLTarget, FootOffsetLLocation, FootOffsetLRotation);
		SetFootOffsets(FName("Enable_FootIK_R"), FName("ik_foot_r"), FName("root"),
			FootOffsetRTarget, FootOffsetRLocation, FootOffsetRRotation);
		SetPelvisIKOffset(FootOffsetLTarget, FootOffsetRTarget);
		break;
	case EMovementState::InAir:
		SetPelvisIKOffset(FVector::ZeroVector, FVector::ZeroVector);
		ResetIKOffsets();
		break;
	default:
		break;;
	}
}

void UAnimInstanceBase::UpdateInAirValues()
{
	FallSpeed = Velocity.Z;
	LandPrediction = CalculateLandPrediction();
	LeanAmount = InterpLeanAmount(LeanAmount, CalculateInAirLeanAmount(), InAirLeanInterpSpeed, DeltaTimeX);
}

void UAnimInstanceBase::UpdateRagdollValues()
{
	FlailRate = UKismetMathLibrary::MapRangeClamped(
		GetOwningComponent()->GetPhysicsLinearVelocity().Size(), 0.0f, 1000.0f, 0.0f, 1.0f);
}

void UAnimInstanceBase::UpdateMovementValues()
{
	VelocityBlend = InterpVelocityBlend(VelocityBlend, CalculateVelocityBlend(), VelocityBlendInterpSpeed, DeltaTimeX);
	DiagonalScaleAmount = CalculateDiagonalScaleAmount();
	RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();
	FLeanAmount TargetLeanAmount;
	TargetLeanAmount.LR = RelativeAccelerationAmount.Y;
	TargetLeanAmount.FB = RelativeAccelerationAmount.X;
	LeanAmount = InterpLeanAmount(LeanAmount, TargetLeanAmount, GroundedLeanInterpSpeed, DeltaTimeX);
	WalkRunBlend = CalculateWalkRunBlend();
	StrideBlend = CalculateStrideBlend();
	StandingPlayRate = CalculateStandingPlayRate();
	CrouchingPlayRate = CalculateCrouchingPlayRate();
}

void UAnimInstanceBase::UpdateRotationValues()
{
	MovementDirection = CalculateMovementDirection();
	float DeltaYaw = (Velocity.Rotation() - CharacterBase->GetControlRotation()).Yaw;
	FVector YawOffsetFBValue = YawOffsetFB->GetVectorValue(DeltaYaw);
	FVector YawOffsetLRValue = YawOffsetLR->GetVectorValue(DeltaYaw);
	FYaw = YawOffsetFBValue.X;
	BYaw = YawOffsetFBValue.Y;
	LYaw = YawOffsetLRValue.X;
	RYaw = YawOffsetLRValue.Y;
}

bool UAnimInstanceBase::ShouldMoveCheck()
{
	return (bIsMoving && bHasMovementInput) || (Speed > 150.0f); 
}

void UAnimInstanceBase::RotateInPlaceCheck()
{
	bRotateL = AimingAngle.X < RotateMinThreshold;
	bRotateR = AimingAngle.X < RotateMaxThreshold;

	if (bRotateL || bRotateR)
	{
		RotateRate = UKismetMathLibrary::MapRangeClamped(AimYawRate, AimYawRateMinRange, AimYawRateMaxRange, MinPlayRate, MaxPlayRate);
	}
}

void UAnimInstanceBase::TurnInPlaceCheck()
{
	bool bCheck = FMath::Abs(AimingAngle.X > TurnCheckMinAngle) && AimYawRate < AimYawRateLimit;
	if (bCheck)
	{
		ElapsedDelayTime += DeltaTimeX;
		float ClampedValue = UKismetMathLibrary::MapRangeClamped(FMath::Abs(AimingAngle.X), TurnCheckMinAngle, 180.0f, MinAngleDelay, MaxAngleDelay);
		if (ElapsedDelayTime > ClampedValue)
		{
			TurnInPlace(FRotator(0.0f, 0.0f, AimingRotation.Yaw), 1.0f, 0.0f, false);
		}
	}
	else
	{
		ElapsedDelayTime = 0.0f;
	}
}

void UAnimInstanceBase::DynamicTransitionCheck()
{
	float FootLDistance = 0.0f;
	// FootLDistance = UkismetAnimationLibrary::K2_DistanceBetweenTwoSocketsAndMapRange(
	// 	GetOwningComponent(), FName("ik_foot_l"), ERelativeTransformSpace::RTS_Component, FName("VB foot_target_l"), 0.0f, 0.0f, 0.0f, 0.0f);
	if (FootLDistance > 8.0f)
	{
		FDynamicMontageParams AnimationParams;
		AnimationParams.Animation = AnimationParams.Animation;
		AnimationParams.BlendInTime = 0.2f;
		AnimationParams.BlendOutTime = 0.2f;
		AnimationParams.PlayRate = 1.5f;
		AnimationParams.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, AnimationParams);
	}

	float FootRDistance = 0.0f;
	// FootRDistance = UkismetAnimationLibrary::K2_DistanceBetweenTwoSocketsAndMapRange(
	// 	GetOwningComponent(), FName("ik_foot_l"), ERelativeTransformSpace::RTS_Component, FName("VB foot_target_l"), 0.0f, 0.0f, 0.0f, 0.0f);
	if (FootRDistance > 8.0f)
	{
		FDynamicMontageParams AnimationParams;
		AnimationParams.Animation = AnimationParams.Animation;
		AnimationParams.BlendInTime = 0.2f;
		AnimationParams.BlendOutTime = 0.2f;
		AnimationParams.PlayRate = 1.5f;
		AnimationParams.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, AnimationParams);
	}
}

bool UAnimInstanceBase::CanRotateInPlace()
{
	return RotationMode == ERotationMode::Aiming || ViewMode == EViewMode::FirstPerson;
}

bool UAnimInstanceBase::CanTurnInPlace()
{
	return RotationMode == ERotationMode::LookingDirection && ViewMode == EViewMode::FirstPerson && GetCurveValue(FName("Enable_Transition")) > 0.99f;
}

bool UAnimInstanceBase::CanDynamicTransition()
{
	return GetCurveValue(FName("Enable_Transition")) == 1.0f;
}

bool UAnimInstanceBase::CanOverlayTransition()
{
	return Stance == EStance::Standing && !bShouldMove;
}

void UAnimInstanceBase::SetFootLocking(FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone, float &CurrentFootLockAlpha,
	FVector &CurrentFootLockLocation, FRotator &CurrentFootLockRotation)
{
	if (GetCurveValue(EnableFootIKCurve) > 0.0f)
	{
		FootLockCurveValue = GetCurveValue(FootLockCurve);
		if (FootLockCurveValue >= 0.99f || FootLockCurveValue < CurrentFootLockAlpha)
		{
			CurrentFootLockAlpha = FootLockCurveValue;
		}

		if (CurrentFootLockAlpha >= 0.99f)
		{
			FTransform SocketTransform = GetOwningComponent()->GetSocketTransform(IKFootBone, ERelativeTransformSpace::RTS_Component);
			CurrentFootLockLocation = SocketTransform.GetLocation();
			CurrentFootLockRotation = SocketTransform.GetRotation().Rotator();
		}

		if (CurrentFootLockAlpha > 0.0f)
		{
			SetFootLockOffsets(CurrentFootLockLocation, CurrentFootLockRotation);
		}
	}
}

void UAnimInstanceBase::SetFootOffsets(FName EnableFootIKCurve, FName IKFootBone, FName RootBone, FVector& CurrentLocationTarget,
	FVector& CurrentLocationOffset, FRotator& CurrentRotationOffset)
{
	if (GetCurveValue(EnableFootIKCurve) > 0.0f)
	{
		CurrentLocationOffset = FVector::ZeroVector;
		CurrentRotationOffset = FRotator::ZeroRotator;
		FVector IKFootBoneLocation = GetOwningComponent()->GetSocketLocation(IKFootBone);
		FVector RootBoneLocation = GetOwningComponent()->GetSocketLocation(RootBone);
		FVector IKFootFloorLocation = FVector(IKFootBoneLocation.X, IKFootBoneLocation.Y, RootBoneLocation.Z);
		FVector Start = IKFootFloorLocation + FVector(0.0, 0.0, IKTraceDistanceAboveFoot);
		FVector End = IKFootFloorLocation - FVector(0.0, 0.0, IKTraceDistanceBelowFoot);
		FHitResult HitResult;
		TArray<AActor*> ActorsToIgnore;
		UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			EDrawDebugTrace::ForOneFrame, HitResult, true, FLinearColor::Red, FLinearColor::Green, 5.0f);
		bool bWalkable = CharacterBase->GetCharacterMovement()->IsWalkable(HitResult);
		FRotator TargetRotationOffset = FRotator::ZeroRotator;
		FVector ImpactPoint = FVector::ZeroVector;
		FVector ImpactNormal = FVector::ZeroVector;
		if (bWalkable)
		{
			ImpactPoint = HitResult.ImpactPoint;
			ImpactNormal = HitResult.ImpactNormal;
			CurrentLocationTarget = ImpactPoint + ImpactNormal * FootHeight - (IKFootFloorLocation + FootHeight * FVector(0, 0, 1.0f));
			TargetRotationOffset = FRotator(-FMath::Atan2(ImpactNormal.X, ImpactNormal.X), 0.0f, FMath::Atan2(ImpactNormal.Y, ImpactNormal.Z));
		}
		
		if (CurrentLocationOffset.Z > CurrentLocationTarget.Z)
		{
			CurrentLocationOffset = FMath::VInterpTo(CurrentLocationOffset, CurrentLocationTarget, DeltaTimeX, 30.0f);
		}
		else
		{
			CurrentLocationOffset = FMath::VInterpTo(CurrentLocationOffset, CurrentLocationTarget, DeltaTimeX, 15.0f);
		}
		
		CurrentRotationOffset = FMath::RInterpTo(CurrentRotationOffset, TargetRotationOffset, DeltaTimeX, 30.0f);
	}
}

void UAnimInstanceBase::SetPelvisIKOffset(FVector FootOffsetLTarget, FVector FootOffsetRTarget)
{
	PelvisAlpha = (GetCurveValue(FName("Enable_FootIK_L")) + GetCurveValue(FName("Enable_FootIK_R"))) / 2.0f;
	if (PelvisAlpha > 0.0f)
	{
		FVector PelvisTarget = FootOffsetLTarget.Z < FootOffsetRTarget.Z ? FootOffsetLTarget : FootOffsetRTarget;
		float InterpSpeed = PelvisTarget.Z > PelvisOffset.Z ? 10.0f : 15.0f;
		PelvisOffset = FMath::VInterpTo(PelvisOffset, PelvisTarget, DeltaTimeX, InterpSpeed);
	}
	else
	{
		PelvisOffset = FVector::ZeroVector;
	}
}

void UAnimInstanceBase::ResetIKOffsets()
{
	FootOffsetLLocation = FMath::VInterpTo(FootOffsetLLocation, FVector::ZeroVector, DeltaTimeX, 15.0f);
	FootOffsetRLocation = FMath::VInterpTo(FootOffsetRLocation, FVector::ZeroVector, DeltaTimeX, 15.0f);
	FootOffsetLRotation = FMath::RInterpTo(FootOffsetLRotation, FRotator::ZeroRotator, DeltaTimeX, 15.0f);
	FootOffsetRRotation = FMath::RInterpTo(FootOffsetRRotation, FRotator::ZeroRotator, DeltaTimeX, 15.0f);
}

float UAnimInstanceBase::CalculateLandPrediction()
{
	if (FallSpeed >= -200.0f)
	{
		return 0.0f;
	}
	FVector Start = CharacterBase->GetCapsuleComponent()->GetComponentLocation();
	FVector End = Start + FVector(Velocity.X, Velocity.Y, FMath::Clamp(Velocity.Z, -4000.0f, -200.0f)).GetUnsafeNormal() *
		UKismetMathLibrary::MapRangeClamped(Velocity.Z, 0.0f, -4000.0f, 50.0f, 2000.0f);
	float Radius = CharacterBase->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float HalfHeight = CharacterBase->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;
	UKismetSystemLibrary::CapsuleTraceSingleByProfile(this, Start, End, Radius, HalfHeight, FName("ALS_Character"), false,
		ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true, FColor::Red, FColor::Green, 5.0f);
	bool bWalkable = CharacterBase->GetCharacterMovement()->IsWalkable(HitResult);
	if (bWalkable && HitResult.bBlockingHit)
	{
		return FMath::Lerp(LandPredictionCurve->GetFloatValue(HitResult.Time), 0.0f, GetCurveValue(FName("Mask_LandPrediction")));
	}
	else
	{
		return 0.0f;
	}
}

FLeanAmount UAnimInstanceBase::InterpLeanAmount(FLeanAmount Current, FLeanAmount Target, float InterpSpeed, float DeltaTime)
{
	FLeanAmount ResultAmount;
	ResultAmount.LR = FMath::FInterpTo(Current.LR, Target.LR, DeltaTime, InterpSpeed);
	ResultAmount.FB = FMath::FInterpTo(Current.FB, Target.FB, DeltaTime, InterpSpeed);
	return ResultAmount;
}

FLeanAmount UAnimInstanceBase::CalculateInAirLeanAmount()
{
	FLeanAmount ResultAmount;
	FVector Lean3d = CharacterBase->GetActorRotation().UnrotateVector(Velocity) / 350.0f;
	FVector2d Lean2d = FVector2d(Lean3d.Y, Lean3d.X);
	FVector2d Speed2d = Lean2d * LeanInAirCurve->GetFloatValue(FallSpeed);
	ResultAmount.LR = Speed2d.X;
	ResultAmount.FB = Speed2d.Y;
	return ResultAmount;
}

FVelocityBlend UAnimInstanceBase::InterpVelocityBlend(FVelocityBlend Current, FVelocityBlend Target, float InterpSpeed, float DeltaTime)
{
	FVelocityBlend ResultBlend;
	ResultBlend.F = FMath::FInterpTo(Current.F, Target.F, DeltaTime, InterpSpeed);
	ResultBlend.B = FMath::FInterpTo(Current.B, Target.B, DeltaTime, InterpSpeed);
	ResultBlend.L = FMath::FInterpTo(Current.L, Target.L, DeltaTime, InterpSpeed);
	ResultBlend.R = FMath::FInterpTo(Current.R, Target.R, DeltaTime, InterpSpeed);
	return ResultBlend;
}

FVelocityBlend UAnimInstanceBase::CalculateVelocityBlend()
{
	Velocity.Normalize();
	FVector LocRelativeVelocityDir = CharacterBase->GetActorRotation().UnrotateVector(Velocity);
	float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) + FMath::Abs(LocRelativeVelocityDir.Z);
	FVector RelativeDirection = LocRelativeVelocityDir / Sum;
	FVelocityBlend ResultBlend;
	ResultBlend.F = FMath::Clamp(RelativeDirection.X, 0.0f, 1.0f);
	ResultBlend.B = FMath::Clamp(RelativeDirection.X, -1.0f, 0.0f);
	ResultBlend.L = FMath::Clamp(RelativeDirection.Y, -1.0f, 0.0f);
	ResultBlend.R = FMath::Clamp(RelativeDirection.Y, 0.0f, 1.0f);
	return ResultBlend;
}

float UAnimInstanceBase::CalculateDiagonalScaleAmount()
{
	return DiagonalScaleAmountCurve->GetFloatValue(FMath::Abs(VelocityBlend.F + VelocityBlend.B));
}

FVector UAnimInstanceBase::CalculateRelativeAccelerationAmount()
{
	UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(CharacterBase->GetMovementComponent());
	if (FVector::DotProduct(Acceleration, Velocity) > 0.0f)
	{
		FVector ClampAcceleration = UKismetMathLibrary::Vector_ClampSizeMax(Acceleration, MovementComponent->GetMaxAcceleration()) / MovementComponent->GetMaxAcceleration();
		return CharacterBase->GetActorRotation().UnrotateVector(ClampAcceleration);
	}
	else
	{
		FVector ClampBrakingAcceleration = UKismetMathLibrary::Vector_ClampSizeMax(Acceleration, MovementComponent->GetMaxBrakingDeceleration()) / MovementComponent->GetMaxBrakingDeceleration();
		return CharacterBase->GetActorRotation().UnrotateVector(ClampBrakingAcceleration);
	}
}

float UAnimInstanceBase::CalculateWalkRunBlend()
{
	switch (Gait)
	{
	case EGait::Running:
		return 0.0f;
	case EGait::Sprinting:
	case EGait::Walking:
		return 1.0f;
	}
	return 0.0f;
}

float UAnimInstanceBase::CalculateStrideBlend()
{
	float InterpSpeed = FMath::Clamp(GetCurveValue(FName("Weight_Gait")) - 1.0f, 0.0f, 1.0f);
	float SpeedLerp = FMath::Lerp(StrideBlendNWalk->GetFloatValue(Speed), StrideBlendNRun->GetFloatValue(Speed), InterpSpeed);
	return FMath::Lerp(SpeedLerp, StrideBlendCWalk->GetFloatValue(Speed), GetCurveValue(FName("BasePose_CLF")));
}

float UAnimInstanceBase::CalculateStandingPlayRate()
{
	float InterpSpeed1 = FMath::Clamp(GetCurveValue(FName("Weight_Gait")) - 1.0f, 0.0f, 1.0f);
	float SpeedLerp = FMath::Lerp(Speed/AnimatedWalkSpeed, Speed/AnimatedRunSpeed, InterpSpeed1);
	float InterpSpeed2 = FMath::Clamp(GetCurveValue(FName("Weight_Gait")) - 2.0f, 0.0f, 1.0f);
	float SppedLerp2 = FMath::Lerp(SpeedLerp, Speed/AnimatedSprintSpeed,  InterpSpeed2);
	return FMath::Clamp((SppedLerp2 / StrideBlend) / GetOwningComponent()->GetComponentScale().Z, 0.0f, 3.0f);
}

float UAnimInstanceBase::CalculateCrouchingPlayRate()
{
	return FMath::Clamp((Speed / AnimatedCrouchSpeed) / StrideBlend / GetOwningComponent()->GetComponentScale().Z, 0.0f, 2.0f);
}

EMovementDirection UAnimInstanceBase::CalculateMovementDirection()
{
	if (Gait == EGait::Walking || Gait == EGait::Running)
	{
		if (RotationMode == ERotationMode::VelocityDirection)
		{
			return EMovementDirection::Forward;
		}

		if (RotationMode == ERotationMode::LookingDirection || RotationMode == ERotationMode::Aiming)
		{
			float Angle = (Velocity.Rotation() - AimingRotation).Yaw;
			return CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Angle);
		}
	}
}

void UAnimInstanceBase::TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent)
{
	float TurnAngle = (TargetRotation - CharacterBase->GetActorRotation()).Yaw;
	FTurnInPlaceAsset TargetTurnAsset;
	if (FMath::Abs(TurnAngle) < Turn180Threshold)
	{
		if (TurnAngle < 0.0f)
		{
			switch (Stance)
			{
			case EStance::Standing:
				TargetTurnAsset = NTurnIPL90;
			case EStance::Crouching:
				TargetTurnAsset = CLFTurnIPL90; 
			}
		}
		else
		{
			switch (Stance)
			{
			case EStance::Standing:
				TargetTurnAsset = NTurnIPR90;
			case EStance::Crouching:
				TargetTurnAsset = CLFTurnIPR90; 
			}
		}
	}
	else
	{
		if (TurnAngle < 0.0f)
		{
			switch (Stance)
			{
			case EStance::Standing:
				TargetTurnAsset = NTurnIPL180;
			case EStance::Crouching:
				TargetTurnAsset = CLFTurnIPL180; 
			}
		}
		else
		{
			switch (Stance)
			{
			case EStance::Standing:
				TargetTurnAsset = NTurnIPR180;
			case EStance::Crouching:
				TargetTurnAsset = CLFTurnIPR180; 
			}
		}
	}
	if (OverrideCurrent || !IsPlayingSlotAnimation(TargetTurnAsset.Animation, TargetTurnAsset.SlotName))
	{
		PlaySlotAnimationAsDynamicMontage(TargetTurnAsset.Animation, TargetTurnAsset.SlotName, 0.2f, 0.2,
			TargetTurnAsset.PlayRate * PlayRateScale, 1, 0.0f, StartTime);
	}
	if (TargetTurnAsset.ScaleTurnAngle)
	{
		RotationScale = (TurnAngle / TargetTurnAsset.AnimatedAngle) * TargetTurnAsset.PlayRate * PlayRateScale;
	}
	else
	{
		RotationScale = TargetTurnAsset.PlayRate * PlayRateScale;
	}
}

void UAnimInstanceBase::SetFootLockOffsets(FVector& LocalLocation, FRotator& LocalRotation)
{
	UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(CharacterBase->GetMovementComponent());
	FRotator RotationDifference = FRotator::ZeroRotator;
	FVector LocationDifference = FVector::ZeroVector;
	if (MovementComponent->IsMovingOnGround())
	{
		RotationDifference = CharacterBase->GetActorRotation() - MovementComponent->GetLastUpdateRotation();
	}
	LocationDifference = GetOwningComponent()->GetComponentRotation().UnrotateVector(Velocity * UGameplayStatics::GetWorldDeltaSeconds(this));
	LocalLocation = (LocalLocation - LocationDifference).RotateAngleAxis(RotationDifference.Yaw, FVector(0.0, 0.0f, -1.0f));
	LocalRotation = LocalRotation - RotationDifference;
}

EMovementDirection UAnimInstanceBase::CalculateQuadrant(EMovementDirection Current, float FRThreshold, float FLThreshold, float BRThreshold, float BLThreshold, float Buffer, float Angle)
{
	bool IncreaseBuffer1 = Current != EMovementDirection::Forward || Current != EMovementDirection::Backward;
	bool IsInRange1 = AngleInRange(Angle, FLThreshold, FRThreshold, Buffer, IncreaseBuffer1);
	if (IsInRange1)
	{
		return EMovementDirection::Forward;
	}

	bool IncreaseBuffer2 = Current != EMovementDirection::Right || Current != EMovementDirection::Left;
	bool IsInRange2 = AngleInRange(Angle, FRThreshold, BRThreshold, Buffer, IncreaseBuffer2);
	if (IsInRange2)
	{
		return EMovementDirection::Right;
	}

	bool IncreaseBuffer3 = Current != EMovementDirection::Right || Current != EMovementDirection::Left;
	bool IsInRange3 = AngleInRange(Angle, BLThreshold, FLThreshold, Buffer, IncreaseBuffer3);
	if (IsInRange3)
	{
		return EMovementDirection::Left;
	}

	return EMovementDirection::Backward;
}

bool UAnimInstanceBase::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer)
{
	return IncreaseBuffer ?
				UKismetMathLibrary::InRange_FloatFloat(Angle, MinAngle - Buffer, MaxAngle + Buffer) :
				UKismetMathLibrary::InRange_FloatFloat(Angle, MinAngle + Buffer, MaxAngle - Buffer);
}