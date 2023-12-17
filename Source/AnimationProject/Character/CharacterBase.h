// Copyright XiaWen, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "XXCharacterMovementComponent.h"
#include "AnimationProject/Common/CommonInterfaces.h"
#include "AnimationProject/Locomotion/LocomotionDefine.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Logging/LogMacros.h"
#include "CharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UTimelineComponent;
struct FInputActionValue;
class UAnimInstanceBase;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ACharacterBase : public ACharacter, public ICharacterInterface, public ICameraInterface
{
	GENERATED_BODY()
	
public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

protected:	
	UPROPERTY(transient, NonTransactional)
	TObjectPtr<UAnimInstanceBase> MainAnimInstance;
	
public:
	ACharacterBase();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void PlayerMovementInput(bool IsForwardAxis);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

private:
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> BodyMesh;
	
public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

#pragma region Locomotion
public:
	// Todo: MovementModelTable, 配置FMovementSettingsState
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* MovementModelDT = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGait DesiredGait = EGait::Running;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERotationMode DesiredRotationMode = ERotationMode::VelocityDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStance DesiredStance = EStance::Standing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EViewMode ViewMode = EViewMode::ThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOverlayState OverlayState = EOverlayState::Default;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UXXCharacterMovementComponent> XXCharacterMovement;
	
private:
	FMovementSettingsState MovementData;
	FMovementSettings CurrentMovementSettings;
	ERotationMode RotationMode = ERotationMode::VelocityDirection;
	FRotator TargetRotation = FRotator::ZeroRotator;
	FRotator LastVelocityRotation = FRotator::ZeroRotator;
	FRotator LastMovementInputRotation = FRotator::ZeroRotator;
	FRotator InAirRotation = FRotator::ZeroRotator;
	EMovementState MovementState = EMovementState::None;
	EMovementState  PreviousMovementState = EMovementState::None;
	EStance Stance = EStance::Standing;
	EStance PreviousStance = EStance::Standing;
	EGait AllowedGait = EGait::Walking;
	EGait ActualGait = EGait::Walking;
	EGait PreviousActualGait = EGait::Running;
	EGait Gait = EGait::Walking;
	EMovementAction MovementAction = EMovementAction::None;
	EMovementAction PreviousMovementAction = EMovementAction::None;
	FMantleTraceSettings FallingTraceSettings;
	FMantleTraceSettings GroundedTraceSettings;
	bool RightShoulder = false;

	FVector Acceleration = FVector::ZeroVector;
	FVector PreviousVelocity = FVector::ZeroVector;
	float Speed = 0.0f;
	bool IsMoving = false;
	float MovementInputAmount = 0.0f;
	bool HasMovementInput = false;
	float PreviousAimYaw = 0.0f;
	float AimYawRate = 0.0f;
	FVector LastRagdollVelocity = FVector::ZeroVector;
	bool RagdollFaceUp = false;
	bool RagdollOnGround = false;
	FMantleParams MantleParams;
	FComponentAndTransform MantleLedgeLS;
	FTransform MantleTarget;
	FTransform MantleActualStartOffset;
	FTransform MantleAnimatedStartOffset;
	UTimelineComponent* TimelineComponent = nullptr;
	UTimelineComponent* MantleTimeline = nullptr;
	bool BreakFall = false;
	float LookUpDownRate = 0.0f;
	float LookLeftRightRate = 0.0f;

private:
	TSoftObjectPtr<UAnimMontage> GetUpBackDefault;
	TSoftObjectPtr<UAnimMontage> GetUpBackLH;
	TSoftObjectPtr<UAnimMontage> GetUpBack2H;
	TSoftObjectPtr<UAnimMontage> GetUpBackRH;
	
	TSoftObjectPtr<UAnimMontage> GetUpFrontDefault;
	TSoftObjectPtr<UAnimMontage> GetUpFrontLH;
	TSoftObjectPtr<UAnimMontage> GetUpFrontRH;
	TSoftObjectPtr<UAnimMontage> GetUpFront2H;

	TSoftObjectPtr<UAnimMontage> LandRollDefault;
	TSoftObjectPtr<UAnimMontage> LandRollLH;
	TSoftObjectPtr<UAnimMontage> LandRollRH;
	TSoftObjectPtr<UAnimMontage> LandRoll2H;

public:
	virtual void BPIGetCurrentStates(
		TEnumAsByte<EMovementMode>& OutPawnMovementMode,
		EMovementState& OutMovementState,
		EMovementState& OutPrevMovementState,
		EMovementAction& OutMovementAction,
		ERotationMode& OutRotationMode,
		EGait& OutActualGait,
		EStance& OutActualStance,
		EViewMode& OutViewMode,
		EOverlayState& OutOverlayState) override;
	virtual void BPIGetEssentialValues(
		FVector& OutVelocity,
		FVector& OutAcceleration,
		FVector& OutMovementInput,
		bool& OutIsMoving,
		bool& OutHasMovementInput,
		float& OutSpeed,
		float& OutMovementInputAmount,
		FRotator& OutAimingRotation,
		float& OutAimYawRate) override;
	virtual void BPISetMovementState(EMovementState NewMovementState) override;
	virtual void BPISetMovementAction(EMovementAction NewMovementAction) override;
	virtual void BPISetRotationMode(ERotationMode NewRotationMode) override;
	virtual void BPISetGait(EGait NewGait) override;
	virtual void BPISetViewMode(EViewMode NewViewMode) override;
	virtual void BPISetOverlayState(EOverlayState NewOverlayState) override;
	virtual FVector BPIGetCameraTarget() override;
	virtual FTransform BPIGet3PPivotTarget() override;
	virtual void BPIGet3PTraceParams(FVector& TraceOrigin, float& TraceRadius, TEnumAsByte<ETraceTypeQuery>& TraceChannel) override;

protected:
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnJumped_Implementation() override;

private:
	void UpdateColoringSystem();
	void UpdateHeldObjectAnimations();
	void UpdateHeldObject();
	void ClearHeldObject();
	void AttachToHand(UStaticMesh* StaticMesh, USkeletalMesh* SkeletalMesh, UClass* NewAnimClass, bool LeftHand, FVector Offset);
	void UpdateLayeringColors();
	void SetDynamicMaterials();
	void SetAndResetColors();

	bool SolidColor = false;
	uint8 ShirtType = 0;
	uint8 PantsType = 0;
	bool Shoes = false;
	bool Gloves = false;
	FLinearColor DefaultColor;
	FLinearColor SkinColor;
	FLinearColor ShirtColor;
	FLinearColor PantsColor;
	FLinearColor ShoesColor;
	FLinearColor GlovesColor;
	FLinearColor OverlayLayerColor;
	FLinearColor AdditiveAmountColor;
	FLinearColor BaseLayerColor;
	FLinearColor HandColor;
	FLinearColor HandIKColor;
	UMaterialInstanceDynamic* Head = nullptr;
	UMaterialInstanceDynamic* Torso = nullptr;
	UMaterialInstanceDynamic* Pelvis = nullptr;
	UMaterialInstanceDynamic* ShoulderL = nullptr;
	UMaterialInstanceDynamic* UpperArmL = nullptr;
	UMaterialInstanceDynamic* LowerArmL = nullptr;
	UMaterialInstanceDynamic* HandL = nullptr;
	UMaterialInstanceDynamic* ShoulderR = nullptr;
	UMaterialInstanceDynamic* UpperArmR = nullptr;
	UMaterialInstanceDynamic* LowerArmR = nullptr;
	UMaterialInstanceDynamic* HandR = nullptr;
	UMaterialInstanceDynamic* UpperLegs = nullptr;
	UMaterialInstanceDynamic* LowerLegs = nullptr;
	UMaterialInstanceDynamic* Feet = nullptr;
	
	FMantleAsset Mantle2mDefault;
	FMantleAsset Mantle1mDefault;
	FMantleAsset Mantle1mLH;
	FMantleAsset Mantle1m2H;
	FMantleAsset Mantle1mRH;
	FMantleAsset Mantle1mBox;

	UStaticMeshComponent* StaticMeshComponent = nullptr;
	USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
	USceneComponent* HeldObjectRoot = nullptr;

private:
	void OnGaitChanged(EGait NewGait);
	void OnOverlayStateChanged(EOverlayState NewOverlayState);
	
	void OnRotationModeChanged(ERotationMode NewRotationMode);
	void SetRotationMode(ERotationMode NewRotationMode);
	
	void OnViewModeChanged(EViewMode NewViewMode);
	void SetViewMode(EViewMode NewViewMode);

	void OnMovementStateChanged(EMovementState NewMovementState);
	void OnMovementActionChanged(EMovementAction NewMovementAction);
	
	void SetEssentialValues();
	void CacheValues();
	void DrawDebugShapes();
	
	void UpdateCharacterMovement();
	void UpdateGroundedRotation();
	float GetAnimCurveValue(FName CurveName);
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed);
	bool CanUpdateMovingRotation();
	void SmoothCharacterRotation(FRotator InTargetRotation, float TargetInterpSpeed, float ActorInterpSpeed);
	void UpdateInAirRotation();
	bool MantleCheck(FMantleTraceSettings TraceSettings, EDrawDebugTrace::Type DebugType);
	void MantleStart(float MantleHeight, FComponentAndTransform MantleLedgeWS, EMantleType MantleType);
	void MantleEnd();
	FMantleAsset GetMantleAsset(EMantleType MantleType);
	EDrawDebugTrace::Type GetTraceDebugType(EDrawDebugTrace::Type ShowTraceType);
	FVector GetCapsuleLocationFromBase(FVector BaseLocation, float ZOffset);
	bool CapsuleHasRoomCheck(
		UCapsuleComponent* CapsuleComp,
		FVector TargetLocation,
		float HeightOffset,
		float RadiusOffset,
		EDrawDebugTrace::Type DebugTye);
	void RagdollStart();
	void RagdollUpdate();
	void RagdollEnd();
	void SetActorLocationDuringRagdoll();
	bool SetLocationAndRotation(
		FVector NewLocation,
		FRotator NewRotation,
		bool bSweep,
		bool bTeleport,
		struct FHitResult& SweepHitResult);
	FVector GetCapsuleBaseLocation(float ZOffset);
	FVector GetPlayerMovementInput();
	void GetControlVector(FVector& ForwardVector, FVector& RightVector);
	float CalculateGroundedRotationRate();
	EGait GetAllowedGait() const;
	EGait GetActualGait(EGait InAllowedGait) const;
	void UpdateDynamicMovementSettings(EGait InAllowedGait);
	bool CanSprint() const;
	FMovementSettings GetTargetMovementSettings() const;
	float GetMappedSpeed() const;
	UAnimMontage* GetRollAnimation();
	void RollEvent();
	void FixDiagonalGamepadValues(float InX, float InY, float& OutX, float& OutY);
	
	UAnimMontage* GetGetUpAnimation(bool bRagdollFaceUp);

#pragma endregion Locomotion
};

