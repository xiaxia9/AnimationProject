// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterBase.h"

#include "AnimInstanceBase.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "SkeletalRenderPublic.h"
#include "XXCharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "AnimationProject/Physics/CollisionChannels.h"
#include "AnimationProject/Player/PlayerControllerBase.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

const FName MovementModelNormalName = "Normal";

//////////////////////////////////////////////////////////////////////////
// ACharacterBase

ACharacterBase::ACharacterBase()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	XXCharacterMovement = CastChecked<UXXCharacterMovementComponent>(GetCharacterMovement());
}

void ACharacterBase::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	if (IsValid(GetMesh()))
	{
		// Make sure the mesh and Animation BluePrint update after the CharacterBP to ensure it gets the most recent values.
		GetMesh()->AddTickPrerequisiteActor(this);
		// Set Reference to the Main Anim Instance.
		MainAnimInstance = Cast<UAnimInstanceBase>(GetMesh()->GetAnimInstance());
	}

	// Set the Movement Model.
	// Get movement data from the Movement Model Data table and set the Movement Data Struct.
	// This allows you to easily switch out movement behaviors.
	FMovementSettingsState* MovementRow = MovementModelDT->FindRow<FMovementSettingsState>(MovementModelNormalName, TEXT("Get Movement Model"));
	if (MovementRow)
	{
		MovementData = *MovementRow;
	}
	
	// Update states to use the initial desired values.
	OnGaitChanged(DesiredGait);
	OnRotationModeChanged(DesiredRotationMode);
	OnViewModeChanged(ViewMode);
	OnOverlayStateChanged(OverlayState);
	if (DesiredStance == EStance::Standing)
	{
		UnCrouch();
	}
	else if (DesiredStance == EStance::Crouching)
	{
		Crouch();
	}

	// Set default rotation values.
	TargetRotation = GetActorRotation();
	LastVelocityRotation = GetActorRotation();
	LastMovementInputRotation = GetActorRotation();
}

void ACharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (IsValid(BodyMesh))
	{
		BodyMesh->SetMasterPoseComponent(GetMesh());
		SetDynamicMaterials();
		SetAndResetColors();
	}
}

void ACharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetEssentialValues();
	switch (MovementState)
	{
	case EMovementState::Grounded:
		UpdateCharacterMovement();
		UpdateGroundedRotation();
		break;
	case EMovementState::InAir:
		UpdateInAirRotation();
		if (HasMovementInput)
		{
			MantleCheck(FallingTraceSettings, EDrawDebugTrace::Type::ForOneFrame);
		}
		break;
	case EMovementState::Ragdoll:
		RagdollUpdate();
		break;
	default:
		break;
	}
	
	CacheValues();
	DrawDebugShapes();

	UpdateColoringSystem();
	UpdateHeldObjectAnimations();
}

void ACharacterBase::UpdateColoringSystem()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(PlayerController))
	{
		APlayerControllerBase* PlayerControllerBase = Cast<APlayerControllerBase>(PlayerController);
		if (IsValid(PlayerControllerBase))
		{
			ACharacter* DebugFocusCharacter = nullptr;
			bool DebugView = false;
			bool ShowHUD = false;
			bool ShowTraces = false;
			bool ShowDebugShapes = false;
			bool ShowLayerColors = false;
			bool Slomo = false;
			bool ShowCharacterInfo = false;

			PlayerControllerBase->BPIGetDebugInfo(DebugFocusCharacter, DebugView, ShowHUD, ShowTraces,
				ShowDebugShapes, ShowLayerColors, Slomo, ShowCharacterInfo);

			if (ShowLayerColors && IsValid(GetMesh()) && GetMesh()->IsVisible())
			{
				UpdateLayeringColors();
			}
			// todo, Do Once
			SetAndResetColors();
		}
	}
}

void ACharacterBase::UpdateHeldObjectAnimations()
{
	if (OverlayState == EOverlayState::Bow)
	{
		// todo cast to bow_animbp
		GetAnimCurveValue(FName("Enable_SpineRotation"));
		// set draw
	}
}

void ACharacterBase::UpdateHeldObject()
{
	switch (OverlayState)
	{
	case EOverlayState::Default:
	case EOverlayState::Masculine:
	case EOverlayState::Feminine:
	case EOverlayState::Injured:
	case EOverlayState::HandsTied:
		ClearHeldObject();
		break;
	case EOverlayState::Rifle:
		AttachToHand(nullptr, nullptr, nullptr, false, FVector::ZeroVector);
		break;
	case EOverlayState::Pistol1H:
		AttachToHand(nullptr, nullptr, nullptr, false, FVector::ZeroVector);
		break;
	case EOverlayState::Pistol2H:
		AttachToHand(nullptr, nullptr, nullptr, false, FVector::ZeroVector);
		break;
	case EOverlayState::Bow:
		AttachToHand(nullptr, nullptr, nullptr, true, FVector::ZeroVector);
		break;
	case EOverlayState::Torch:
		AttachToHand(nullptr, nullptr, nullptr, true, FVector::ZeroVector);
		break;
	case EOverlayState::Binoculars:
		AttachToHand(nullptr, nullptr, nullptr, true, FVector::ZeroVector);
		break;
	case EOverlayState::Box:
		AttachToHand(nullptr, nullptr, nullptr, true, FVector::ZeroVector);
		break;
	case EOverlayState::Barrel:
		AttachToHand(nullptr, nullptr, nullptr, true, FVector::ZeroVector);
		break;
	}
}

void ACharacterBase::ClearHeldObject()
{
	if (IsValid(StaticMeshComponent) && IsValid(SkeletalMeshComponent))
	{
		StaticMeshComponent->SetStaticMesh(nullptr);
		SkeletalMeshComponent->SetSkeletalMesh(nullptr);
		SkeletalMeshComponent->SetAnimInstanceClass(nullptr);
	}
}

void ACharacterBase::AttachToHand(UStaticMesh* StaticMesh, USkeletalMesh* SkeletalMesh, UClass* NewAnimClass, bool LeftHand, FVector Offset)
{
	ClearHeldObject();
	
	if (IsValid(StaticMesh))
	{
		StaticMeshComponent->SetStaticMesh(StaticMesh);
	}
	
	if (IsValid(SkeletalMesh))
	{
		SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);
	}
	
	if (IsValid(NewAnimClass))
	{
		SkeletalMeshComponent->SetAnimInstanceClass(NewAnimClass);
	}

	if (IsValid(HeldObjectRoot))
	{
		FName SocketName = LeftHand ? FName("VB RHS_ik_hand_Gun") : FName("VB LHS_ik_hand_Gun");
		HeldObjectRoot->K2_AttachToComponent(GetMesh(), SocketName, EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		HeldObjectRoot->SetRelativeLocation(Offset);
	}
}

void ACharacterBase::UpdateLayeringColors()
{
	auto SetParameter1 = [this](UMaterialInstanceDynamic* Part, FName AdditiveCurve, FName BaseCurve)
	{
		FLinearColor AdditiveColor = UKismetMathLibrary::LinearColorLerp(
			OverlayLayerColor, AdditiveAmountColor, GetAnimCurveValue(AdditiveCurve));
		FLinearColor BaseColor = UKismetMathLibrary::LinearColorLerp(
			BaseLayerColor, AdditiveColor, GetAnimCurveValue(BaseCurve));
		Part->SetVectorParameterValue(FName("BaseColor"), BaseColor);
	};

	auto SetParameter2 = [this](UMaterialInstanceDynamic* Part, FName Curve)
	{
		FLinearColor Color = UKismetMathLibrary::LinearColorLerp(
			BaseLayerColor, AdditiveAmountColor, GetAnimCurveValue(Curve));
		Part->SetVectorParameterValue(FName("BaseColor"), Color);
	};
	
	SetParameter1(Head, FName("Layering_Head_Add"), FName("Layering_Head"));
	SetParameter1(Torso, FName("Layering_Spine_Add"), FName("Layering_Spine"));
	SetParameter2(Pelvis, FName("Layering_Pelvis"));
	SetParameter2(UpperLegs, FName("Layering_Legs"));
	SetParameter2(LowerLegs, FName("Layering_Legs"));
	SetParameter2(Feet, FName("Layering_Legs"));
	
	SetParameter1(ShoulderL, FName("Layering_Arm_L_Add"), FName("Layering_Arm_L"));
	SetParameter1(UpperArmL, FName("Layering_Arm_L_Add"), FName("Layering_Arm_L"));
	SetParameter1(LowerArmL, FName("Layering_Arm_L_Add"), FName("Layering_Arm_L"));
	FLinearColor ArmLColor;
	LowerArmL->GetVectorParameterValue(FName("BaseColor"), ArmLColor);
	FLinearColor AdditiveColor = UKismetMathLibrary::LinearColorLerp(
			ArmLColor, HandColor, GetAnimCurveValue(FName("Layering_Hand_L")));
	FLinearColor HandLColor = UKismetMathLibrary::LinearColorLerp(
			AdditiveColor, HandIKColor, GetAnimCurveValue(FName("Enable_HandIK_L")));
	HandL->GetVectorParameterValue(FName("BaseColor"), HandLColor);
	
	SetParameter1(ShoulderR, FName("Layering_Arm_R_Add"), FName("Layering_Arm_R"));
	SetParameter1(UpperArmR, FName("Layering_Arm_R_Add"), FName("Layering_Arm_R"));
	SetParameter1(LowerArmR, FName("Layering_Arm_R_Add"), FName("Layering_Arm_R"));
	FLinearColor ArmRColor;
	LowerArmR->GetVectorParameterValue(FName("BaseColor"), ArmRColor);
	FLinearColor AdditiveRColor = UKismetMathLibrary::LinearColorLerp(
			ArmRColor, HandColor, GetAnimCurveValue(FName("Layering_Hand_L")));
	FLinearColor HandRColor = UKismetMathLibrary::LinearColorLerp(
			AdditiveRColor, HandIKColor, GetAnimCurveValue(FName("Enable_HandIK_L")));
	HandR->GetVectorParameterValue(FName("BaseColor"), HandRColor);
}

void ACharacterBase::SetDynamicMaterials()
{
	Head = GetMesh()->CreateDynamicMaterialInstance(2);
	Torso = GetMesh()->CreateDynamicMaterialInstance(1);
	Pelvis = GetMesh()->CreateDynamicMaterialInstance(0);
	UpperLegs = GetMesh()->CreateDynamicMaterialInstance(6);
	LowerLegs = GetMesh()->CreateDynamicMaterialInstance(7);
	Feet = GetMesh()->CreateDynamicMaterialInstance(8);
	ShoulderL = GetMesh()->CreateDynamicMaterialInstance(4);
	UpperArmL = GetMesh()->CreateDynamicMaterialInstance(3);
	LowerArmL = GetMesh()->CreateDynamicMaterialInstance(5);
	ShoulderR = GetMesh()->CreateDynamicMaterialInstance(12);
	UpperArmR = GetMesh()->CreateDynamicMaterialInstance(11);
	LowerArmR = GetMesh()->CreateDynamicMaterialInstance(13);
}

void ACharacterBase::SetAndResetColors()
{
	if (SolidColor)
	{
		Head->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		Torso->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		Pelvis->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		ShoulderL->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		UpperArmL->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		LowerArmL->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		HandL->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		ShoulderR->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		UpperArmR->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		LowerArmR->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		HandR->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		UpperLegs->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		LowerLegs->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		Feet->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
		Head->SetVectorParameterValue(FName("BaseColor"), DefaultColor);
	}
	else
	{
		Head->SetVectorParameterValue(FName("BaseColor"), SkinColor);
		switch (ShirtType)
		{
		case 0:
			Torso->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			ShoulderL->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			ShoulderR->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			UpperArmL->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			UpperArmR->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			LowerArmL->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			LowerArmR->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			break;
		case 1:
			Torso->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			ShoulderL->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			ShoulderR->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			UpperArmL->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			UpperArmR->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			LowerArmL->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			LowerArmR->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			break;
		case 2:
			Torso->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			ShoulderL->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			ShoulderR->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			UpperArmL->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			UpperArmR->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			LowerArmL->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			LowerArmR->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			break;
		case 3:
			Torso->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			ShoulderL->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			ShoulderR->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			UpperArmL->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			UpperArmR->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			LowerArmL->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			LowerArmR->SetVectorParameterValue(FName("BaseColor"), ShirtColor);
			break;
		default:
			break;
		}

		switch (PantsType)
		{
		case 0:
			Pelvis->SetVectorParameterValue(FName("BaseColor"), PantsColor);
			UpperLegs->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			LowerLegs->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			break;
		case 1:
			Pelvis->SetVectorParameterValue(FName("BaseColor"), PantsColor);
			UpperLegs->SetVectorParameterValue(FName("BaseColor"), PantsColor);
			LowerLegs->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			break;
		case 2:
			Pelvis->SetVectorParameterValue(FName("BaseColor"), PantsColor);
			UpperLegs->SetVectorParameterValue(FName("BaseColor"), PantsColor);
			LowerLegs->SetVectorParameterValue(FName("BaseColor"), PantsColor);
			break;
		default:
			break;
		}

		if (Shoes)
		{
			Feet->SetVectorParameterValue(FName("BaseColor"), ShoesColor);
		}
		else
		{
			Feet->SetVectorParameterValue(FName("BaseColor"), SkinColor);
		}

		if (Gloves)
		{
			HandL->SetVectorParameterValue(FName("BaseColor"), GlovesColor);
			HandR->SetVectorParameterValue(FName("BaseColor"), GlovesColor);
		}
		else
		{
			HandL->SetVectorParameterValue(FName("BaseColor"), SkinColor);
			HandR->SetVectorParameterValue(FName("BaseColor"), SkinColor);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharacterBase::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACharacterBase::Look);

		// MoveForward/Backwards
		// PlayerMovementInput(true);

		// MoveRight/Left
		// PlayerMovementInput(false);

		// LookUp/Down
		// AddControllerPitchInput(LookUpDownRate * GetInputAxisValue(""));

		// LookLeft/Right
		// AddControllerYawInput(LookLeftRightRate * GetInputAxisValue(""));

		// JumpAction
		// Pressed
		// if (MovementAction == EMovementAction::None)
		// {
		// 	switch (MovementState)
		// 	{
		// 	case EMovementState::None:
		// 	case EMovementState::Grounded:
		// 	case EMovementState::InAir:
		// 		if (MovementState == EMovementState::Grounded)
		// 		{
		// 			bool bCanClimb =
		// 				HasMovementInput ? MantleCheck(GroundedTraceSettings, EDrawDebugTrace::Type::ForDuration) : false;
		// 			if (bCanClimb == false || !HasMovementInput)
		// 			{
		// 				if (Stance == EStance::Standing)
		// 				{
		// 					Jump();
		// 				}
		// 				else if (Stance == EStance::Crouching)
		// 				{
		// 					UnCrouch();
		// 				}
		// 			}
		// 		}
		// 		break;
		// 		
		// 	case EMovementState::Ragdoll:
		// 		RagdollEnd();
		// 		break;
		// 		
		// 	default:
		// 		break;
		// 	}
		// }

		// Released
		// StopJumping();

		// StanceAction
		// if (MovementAction == EMovementAction::None)
		// {
		// 	// todo1, Multi Tap Input, 判断按压次数
		// 	// First Press
		// 	if (MovementState == EMovementState::Grounded)
		// 	{
		// 		if (Stance == EStance::Standing)
		// 		{
		// 			DesiredStance = EStance::Crouching;
		// 			Crouch();
		// 		}
		// 		else if (Stance == EStance::Crouching)
		// 		{
		// 			DesiredStance = EStance::Standing;
		// 			UnCrouch();
		// 		}
		// 	}
		// 	else if (MovementState == EMovementState::InAir)
		// 	{
		// 		BreakFall = true;
		// 		// todo1, Retriggerable Delay
		// 		BreakFall = false;
		// 	}
		// 	// multi press
		// 	RollEvent();
		// 	if (Stance == EStance::Standing)
		// 	{
		// 		DesiredStance = EStance::Crouching;
		// 	}
		// 	else if (Stance == EStance::Crouching)
		// 	{
		// 		DesiredStance = EStance::Standing;
		// 	}
		// }

		// WalkAction
		// if (DesiredGait == EGait::Walking)
		// {
		// 	DesiredGait = EGait::Running;
		// }
		// else if (DesiredGait == EGait::Running)
		// {
		// 	DesiredGait = EGait::Walking;
		// }

		// SprintAction
		// Pressed
		// DesiredGait = EGait::Sprinting;
		// Released
		// DesiredGait = EGait::Running;

		// SelectRotationMode_1
		// DesiredRotationMode = ERotationMode::VelocityDirection;
		// BPISetRotationMode(DesiredRotationMode);

		// SelectRotationMode_2
		// DesiredRotationMode = ERotationMode::LookingDirection;
		// BPISetRotationMode(DesiredRotationMode);

		// AimAction
		// Pressed
		// BPISetRotationMode(ERotationMode::Aiming);
		// Released
		// if (ViewMode == EViewMode::ThirdPerson)
		// {
		// 	BPISetRotationMode(DesiredRotationMode);
		// }
		// else if (ViewMode == EViewMode::FirstPerson)
		// {
		// 	BPISetRotationMode(ERotationMode::LookingDirection);
		// }

		// CameraAction
		// todo1, HoldInput
		// Pressed || Released
		// Held
		// if (ViewMode == EViewMode::ThirdPerson)
		// {
		// 	BPISetViewMode(EViewMode::FirstPerson);
		// }
		// else if (ViewMode == EViewMode::FirstPerson)
		// {
		// 	BPISetViewMode(EViewMode::ThirdPerson);
		// }
		// Tapped
		// RightShoulder = !RightShoulder;

		// RagdollAction
		// switch (MovementState)
		// {
		// case EMovementState::None:
		// case EMovementState::Grounded:
		// case EMovementState::InAir:
		// case EMovementState::Mantling:
		// 	RagdollStart();
		// 	break;
		// case EMovementState::Ragdoll:
		// 	RagdollEnd();
		// 	break;
		// default:
		// 	break;
		// }

		// M
		// Pressed
		// todo Flip Flop
		// BodyMesh->SetSkeletalMesh(nullptr, true);
		// GetMesh()->SetVisibility(false, false);
		// BodyMesh->SetSkeletalMesh(nullptr, true);
		// GetMesh()->SetVisibility(true, false);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACharacterBase::PlayerMovementInput(bool IsForwardAxis)
{
	if (MovementState == EMovementState::Grounded ||
		MovementState == EMovementState::InAir)
	{
		FVector ForwardVector;
		FVector RightVector;
		GetControlVector(ForwardVector, RightVector);
		//todo, Get MoveForward/Backwards, Get MoveRight/Left
		float OutX, OutY;
		FixDiagonalGamepadValues(0.0f, 0.0f, OutX, OutY);
		if (IsForwardAxis)
		{
			AddMovementInput(ForwardVector, OutY);
		}
		else
		{
			AddMovementInput(ForwardVector, OutX);
		}
	}
}

void ACharacterBase::FixDiagonalGamepadValues(float InX, float InY, float& OutX, float& OutY)
{
	float ClampX = UKismetMathLibrary::MapRangeClamped(
		FMath::Abs(InX), 0.0f, 0.6f, 1.0f, 1.2f);
	float ClampY = UKismetMathLibrary::MapRangeClamped(
		FMath::Abs(InY), 0.0f, 0.6f, 1.0f, 1.2f);
	OutX = UKismetMathLibrary::FClamp(InX * ClampY, -1.0f, 1.0f);
	OutY = UKismetMathLibrary::FClamp(InY * ClampX, -1.0f, 1.0f);
}

void ACharacterBase::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACharacterBase::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACharacterBase::OnOverlayStateChanged(EOverlayState NewOverlayState)
{
	OverlayState = NewOverlayState;
	UpdateHeldObject();
}

void ACharacterBase::OnRotationModeChanged(ERotationMode NewRotationMode)
{
	RotationMode = NewRotationMode;
	if (RotationMode == ERotationMode::VelocityDirection
		&& ViewMode == EViewMode::FirstPerson)
	{
		SetViewMode(EViewMode::ThirdPerson);
	}
}

void ACharacterBase::SetRotationMode(ERotationMode NewRotationMode)
{
	if (NewRotationMode != RotationMode)
	{
		OnRotationModeChanged(NewRotationMode);
	}
}

void ACharacterBase::OnViewModeChanged(EViewMode NewViewMode)
{
	ViewMode = NewViewMode;
	if (ViewMode == EViewMode::ThirdPerson)
	{
		if (RotationMode == ERotationMode::VelocityDirection
			|| RotationMode == ERotationMode::LookingDirection)
		{
			SetRotationMode(DesiredRotationMode);
		}
	}
	else if (ViewMode == EViewMode::FirstPerson)
	{
		if (RotationMode == ERotationMode::VelocityDirection)
		{
			SetRotationMode(ERotationMode::LookingDirection);
		}
	}
}

void ACharacterBase::SetViewMode(EViewMode NewViewMode)
{
	if (NewViewMode != ViewMode)
	{
		OnViewModeChanged(NewViewMode);
	}
}

void ACharacterBase::SetEssentialValues()
{
	// How the capsule is Moving
	// Function: CalculateAcceleration
	Acceleration = (GetVelocity() - PreviousVelocity)/UGameplayStatics::GetWorldDeltaSeconds(this);
	Speed = GetVelocity().Size2D();
	IsMoving = Speed > 1.0;
	if (IsMoving)
	{
		LastVelocityRotation = GetVelocity().Rotation();
	}

	if(IsValid(XXCharacterMovement))
	{
		MovementInputAmount = XXCharacterMovement->GetCurrentAcceleration().Length() /  XXCharacterMovement->MaxAcceleration;
		HasMovementInput = MovementInputAmount > 0.0;
		if (HasMovementInput)
		{
			LastMovementInputRotation = XXCharacterMovement->GetCurrentAcceleration().Rotation();
		}
	}

	AimYawRate = FMath::Abs((GetControlRotation().Yaw - PreviousAimYaw) / UGameplayStatics::GetWorldDeltaSeconds(this));
}

void ACharacterBase::CacheValues()
{
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = GetControlRotation().Yaw;
}

void ACharacterBase::DrawDebugShapes()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(PlayerController))
	{
		APlayerControllerBase* PlayerControllerBase = Cast<APlayerControllerBase>(PlayerController);
		if (IsValid(PlayerControllerBase))
		{
			ACharacter* DebugFocusCharacter = nullptr;
			bool DebugView = false;
			bool ShowHUD = false;
			bool ShowTraces = false;
			bool ShowDebugShapes = false;
			bool ShowLayerColors = false;
			bool Slomo = false;
			bool ShowCharacterInfo = false;

			PlayerControllerBase->BPIGetDebugInfo(DebugFocusCharacter, DebugView, ShowHUD, ShowTraces,
				ShowDebugShapes, ShowLayerColors, Slomo, ShowCharacterInfo);
			
			if (ShowDebugShapes && IsValid(XXCharacterMovement) && IsValid(GetMesh()))
			{
				// todo, 区分不同颜色
				// Velocity Arrow
				FVector CurrentVelocity = GetVelocity();
				FVector SelectVelocity = CurrentVelocity.IsNearlyZero() ? LastVelocityRotation.Vector() : CurrentVelocity;
				FVector OffsetVelocity = SelectVelocity.GetUnsafeNormal() * UKismetMathLibrary::MapRangeClamped(
					CurrentVelocity.Length(), 0.0f, XXCharacterMovement->MaxWalkSpeed, 50.0f, 75.0f);
				FVector LineStart = GetActorLocation() - FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
				FVector LineEnd = LineStart + OffsetVelocity;
				UKismetSystemLibrary::DrawDebugArrow(
					this, LineStart, LineEnd, 60.0f, FColor::Red, 0.0f, 5.0f);

				// Movement Input Arrow
				FVector CurrentAcceleration = XXCharacterMovement->GetCurrentAcceleration();
				FVector SelectAcceleration = CurrentAcceleration.IsNearlyZero() ? LastMovementInputRotation.Vector() : CurrentAcceleration;
				FVector OffsetAcceleration = SelectAcceleration.GetUnsafeNormal() * UKismetMathLibrary::MapRangeClamped(
					CurrentVelocity.Length() / XXCharacterMovement->GetMaxAcceleration(),
					0.0f, 1.0f, 50.0f, 75.0f);
				FVector AccelerationLineStart = GetActorLocation() - FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 3.5f);
				FVector AccelerationLineEnd = AccelerationLineStart + OffsetAcceleration;
				UKismetSystemLibrary::DrawDebugArrow(
					this, AccelerationLineStart, AccelerationLineEnd, 50.0f, FColor::Yellow, 0.0f, 3.0f);

				// Target Rotation Arrow
				FVector TargetRotationStart = GetActorLocation() - FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 7.0f);
				FVector TargetRotationEnd = TargetRotationStart + TargetRotation.Vector().GetUnsafeNormal() * 50.0f;
				UKismetSystemLibrary::DrawDebugArrow(
					this, TargetRotationStart, TargetRotationEnd, 50.0f, FColor::Blue, 0.0f, 3.0f);

				// Aiming Rotation Cone
				FVector AimingRotationOrigin = GetMesh()->GetSocketLocation(FName("FP_Camera"));
				FVector AimingRotationDirection = GetControlRotation().Vector().GetUnsafeNormal();
				UKismetSystemLibrary::DrawDebugCone(
					this, AimingRotationOrigin, AimingRotationDirection, 100.0f, 30.f,
					30.0f, 8, FColor::Blue, 0.0f, 0.5f);;

				// Capsule
				UKismetSystemLibrary::DrawDebugCapsule(this, GetActorLocation(),
					GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
					GetCapsuleComponent()->GetScaledCapsuleRadius(),
					GetActorRotation(), FColor::Black, 0.0f, 0.3f);
			}
		}
	}
	return;
}

void ACharacterBase::UpdateCharacterMovement()
{
	AllowedGait = GetAllowedGait();
	
	ActualGait = GetActualGait(AllowedGait);
	if (ActualGait != Gait)
	{
		BPISetGait(ActualGait);
	}

	UpdateDynamicMovementSettings(AllowedGait);
}

void ACharacterBase::UpdateInAirRotation()
{
	// 优化，根据模式提供接口，抽象主要接口，不要在每个函数里都调用判断
	if (RotationMode == ERotationMode::VelocityDirection ||
		RotationMode == ERotationMode::LookingDirection)
	{
		SmoothCharacterRotation(FRotator(0.0f, InAirRotation.Yaw, 0.0f), 0.0f, 5.0f);
	}
	else if (RotationMode == ERotationMode::Aiming)
	{
		SmoothCharacterRotation(FRotator(0.0f, GetControlRotation().Yaw, 0.0f), 0.0f, 15.0f);
		InAirRotation = GetActorRotation();
	}
}

bool ACharacterBase::MantleCheck(FMantleTraceSettings TraceSettings, EDrawDebugTrace::Type DebugType)
{
	// Can Climb/Vault
	// Step 1, 向前追踪以找到角色无法行走的墙/对象。
	FVector InitialTraceImpactPoint = FVector::ZeroVector;
	FVector InitialTraceNormal = FVector::ZeroVector;
	float LedegVectorZ = (TraceSettings.MaxLedgeHeight + TraceSettings.MinLedgeHeight) / 2.0f;
	FVector CalpsuleLocation = GetCapsuleBaseLocation(2.0) + GetPlayerMovementInput() * -30.0f;
	FVector BlockStart = CalpsuleLocation + FVector(0, 0, LedegVectorZ);
	FVector BlockEnd = BlockStart + GetPlayerMovementInput() * TraceSettings.ReachDistance;
	float HalfHeight = (TraceSettings.MaxLedgeHeight - TraceSettings.MinLedgeHeight) / 2.0f + 1.0f;
	FHitResult BlockHitResult;
	TArray<AActor*> ActorsToIgnore;
	// Todo, TraceChannel -> Climbable
	UKismetSystemLibrary::CapsuleTraceSingle(
		this, BlockStart, BlockEnd, TraceSettings.ForwardTraceRadius,
		HalfHeight, TraceTypeQuery1,false, ActorsToIgnore,
		GetTraceDebugType(DebugType), BlockHitResult, true,
		FLinearColor::Black, FLinearColor::Black, 1.0f);
	if (!XXCharacterMovement->IsWalkable(BlockHitResult)
		&& BlockHitResult.bBlockingHit
		&& !BlockHitResult.bStartPenetrating)
	{
		InitialTraceImpactPoint = BlockHitResult.ImpactPoint;
		InitialTraceNormal = BlockHitResult.Normal;
	}
	else
	{
		return false;
	}

	// Step 2, 从第一个轨迹的撞击点向下追踪，并确定撞击位置是否可行走。
	FVector DownTraceLocation = FVector::ZeroVector;
	FVector Location = FVector(InitialTraceImpactPoint.X, InitialTraceImpactPoint.Y, GetCapsuleBaseLocation(2.0).Z);
	FVector CanWalkableEnd = Location + InitialTraceNormal * 15.0f;
	FVector CanWalkableStart = CanWalkableEnd +
		FVector(0.0f, 0.0f, TraceSettings.MaxLedgeHeight + TraceSettings.DownwardTraceRadius + 1.0f);
	FHitResult Step2HitResult;
    TArray<AActor*> Step2ActorsToIgnore;
	UPrimitiveComponent* HitComponent = nullptr;
    // Todo, TraceChannel -> Climbable
	UKismetSystemLibrary::CapsuleTraceSingle(
		this, CanWalkableStart, CanWalkableEnd, TraceSettings.DownwardTraceRadius,
		HalfHeight, TraceTypeQuery1,false, Step2ActorsToIgnore,
		GetTraceDebugType(DebugType), Step2HitResult, true,
		FLinearColor::Yellow, FLinearColor::Red, 1.0f);
	if (XXCharacterMovement->IsWalkable(Step2HitResult) && Step2HitResult.bBlockingHit)
	{
		DownTraceLocation = FVector(Step2HitResult.Location.X, Step2HitResult.Location.Y, Step2HitResult.ImpactPoint.Z);
		HitComponent = Step2HitResult.Component.Get();
	}
	else
	{
		return false;
	}

	// Step3, 检查胶囊在向下轨迹的位置是否有空间站立。如果是，请将该位置设置为“目标变换”，并计算地幔高度。
	FRotator Rotation = (InitialTraceNormal * FVector(-1.0f, -1.0f, 0.0f)).Rotation();
	FVector BaseLocation = GetCapsuleLocationFromBase(DownTraceLocation, 2.0f);
	FTransform TargetTransform;
	float MantleHeight = 0.0f;
	bool HasRoomCheck = CapsuleHasRoomCheck(
		GetCapsuleComponent(),
		GetCapsuleLocationFromBase(DownTraceLocation, 2.0f),
		0.0f, 0.0f, GetTraceDebugType(DebugType));
	if (HasRoomCheck)
	{
		FRotator BaseRotation = (InitialTraceNormal * FVector(-1.0f, -1.0f, 0.0f)).Rotation();
		TargetTransform = FTransform(Rotation, BaseLocation, FVector::OneVector);
		MantleHeight = (TargetTransform.GetLocation() - GetActorLocation()).Z;
	}
	else
	{
		return false;
	}

	// Step4, 通过检查移动模式和攀爬高度来确定攀爬类型。
	EMantleType MantleType = EMantleType::HighMantle;
	switch (MovementState)
	{
	case EMovementState::None:
	case EMovementState::Grounded:
	case EMovementState::Mantling:
	case EMovementState::Ragdoll:
		if (MantleHeight > 125.0f)
		{
			MantleType = EMantleType::HighMantle;
		}
		else
		{
			MantleType = EMantleType::LowMantle;
		}
		break;
	case EMovementState::InAir:
		MantleType = EMantleType::FallingCatch;
		break;
	default:
		break;
	}

	// Step5, 如果一切顺利，启动攀爬
	FComponentAndTransform MantleLedgeWS;
	MantleLedgeWS.Transform = TargetTransform;
	MantleLedgeWS.Component = HitComponent;
	MantleStart(MantleHeight, MantleLedgeWS, MantleType);
	return true;
}

void ACharacterBase::MantleStart(float MantleHeight, FComponentAndTransform MantleLedgeWS, EMantleType MantleType)
{
	// Step1, 获取攀爬资源并使用它来设置新的攀爬参数。
	FMantleAsset MantleAsset = GetMantleAsset(MantleType);
	MantleParams.AnimMontage = MantleAsset.AnimMontage;
	MantleParams.PositionCurve = MantleAsset.PositionCurve;
	MantleParams.PlayRate = UKismetMathLibrary::MapRangeClamped(MantleHeight, MantleAsset.LowHeight,
		MantleAsset.HighHeight, MantleAsset.LowPlayRate, MantleAsset.HighPlayRate);
	MantleParams.StartingPosition = UKismetMathLibrary::MapRangeClamped(MantleHeight, MantleAsset.LowHeight,
	MantleAsset.HighHeight, MantleAsset.LowStartPosition, MantleAsset.HightStartPosition);
	MantleParams.StartingOffset = MantleAsset.StartingOffset;

	// Step2, 将世界空间目标转换为攀爬组件的局部空间，用于移动对象。
	MantleLedgeLS.Transform = MantleLedgeWS.Transform * MantleLedgeWS.Component->GetComponentTransform().Inverse();
	MantleLedgeLS.Component = MantleLedgeWS.Component;

	// Step3, 设置“Mantle Target”并计算“Starting Offset”（演员和目标变换之间的偏移量）。
	MantleTarget = MantleLedgeLS.Transform;
	FTransform ActorTransform = GetActorTransform();
	MantleActualStartOffset.SetLocation(ActorTransform.GetLocation() - MantleTarget.GetLocation());
	MantleActualStartOffset.SetRotation(ActorTransform.GetRotation() - MantleTarget.GetRotation());
	MantleActualStartOffset.SetScale3D(ActorTransform.GetScale3D() - MantleTarget.GetScale3D());

	// Step4, 计算从目标位置开始的动画偏移。这将是实际动画相对于“目标变换”开始的位置。
	FVector MantleOffset = FVector(MantleTarget.GetRotation().Vector() * MantleParams.StartingOffset.Y);
	FVector OriginLocation = MantleTarget.GetLocation() - FVector(MantleOffset.X, MantleOffset.Y, MantleParams.StartingOffset.Z);
	MantleAnimatedStartOffset.SetLocation(OriginLocation - MantleTarget.GetLocation());
	MantleAnimatedStartOffset.SetRotation(MantleTarget.GetRotation() - MantleTarget.GetRotation());
	MantleAnimatedStartOffset.SetScale3D(FVector(1.0f, 1.0f, 1.0f) - MantleTarget.GetScale3D());

	// step5, 清除角色移动模式，并将移动状态设置为“Climb”
	// todo 可以优化
	XXCharacterMovement->SetMovementMode(EMovementMode::MOVE_None);
	BPISetMovementState(EMovementState::Mantling);

	// step6, 配置Mantle Timeline，使其长度与Lerp/Correction曲线减去起始位置的长度相同，并以与动画相同的速度播放。然后开始时间线。
	// Todo, GetTimeRange()
	// MantleParams.PositionCurve->GetTimeRange()
	float MaxTime = 0.0f;
	if (IsValid(TimelineComponent))
	{
		TimelineComponent->SetTimelineLength(MaxTime - MantleParams.StartingPosition);
		TimelineComponent->SetPlayRate(MantleParams.PlayRate);
		TimelineComponent->PlayFromStart();
	}

	// step7, 如果有效，播放动画蒙太奇。
	if (IsValid(MantleParams.AnimMontage) && IsValid(MainAnimInstance))
	{
		MainAnimInstance->Montage_Play(MantleParams.AnimMontage, MantleParams.PlayRate,
			EMontagePlayReturnType::MontageLength, MantleParams.StartingPosition, false);
	}

	if (MantleType == EMantleType::HighMantle ||
		MantleType == EMantleType::FallingCatch)
	{
		ClearHeldObject();
	}
}

void ACharacterBase::MantleEnd()
{
	XXCharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking);
	UpdateHeldObject();
}

FMantleAsset ACharacterBase::GetMantleAsset(EMantleType MantleType)
{
	// // Todo, 创建初始化结构体，直接初始化
	// FMantleAsset MantleAsset;
	// switch (MantleType)
	// {
	// case EMantleType::HighMantle:
	// 	// MantleAsset.PositionCurve;
	// 	MantleAsset.StartingOffset = FVector(0.0f, 65.0f, 200.0f);
	// 	MantleAsset.LowHeight = 50.f;
	// 	MantleAsset.LowPlayRate = 1.0f;
	// 	MantleAsset.LowStartPosition = 0.5f;
	// 	MantleAsset.HighHeight = 100.0f;
	// 	MantleAsset.HighPlayRate = 1.0f;
	// 	MantleAsset.HightStartPosition = 0.0f;
	// 	break;
	// case EMantleType::LowMantle:
	// 	// MantleAsset.PositionCurve;
	// 	MantleAsset.StartingOffset = FVector(0.0f, 65.0f, 200.0f);
	// 	MantleAsset.LowHeight = 125.f;
	// 	MantleAsset.LowPlayRate = 1.0f;
	// 	MantleAsset.LowStartPosition = 0.6f;
	// 	MantleAsset.HighHeight = 200.0f;
	// 	MantleAsset.HighPlayRate = 1.2f;
	// 	MantleAsset.HightStartPosition = 0.0f;
	// 	break;
	// case EMantleType::FallingCatch:
	// 	// MantleAsset.PositionCurve;
	// 	MantleAsset.StartingOffset = FVector(0.0f, 65.0f, 200.0f);
	// 	MantleAsset.LowHeight = 125.f;
	// 	MantleAsset.LowPlayRate = 1.2f;
	// 	MantleAsset.LowStartPosition = 0.6f;
	// 	MantleAsset.HighHeight = 200.0f;
	// 	MantleAsset.HighPlayRate = 1.2f;
	// 	MantleAsset.HightStartPosition = 0.0f;
	// 	break;
	// }
	// return MantleAsset;

	FMantleAsset ResultAsset;
	if (MantleType == EMantleType::HighMantle ||
		MantleType == EMantleType::FallingCatch)
	{
		ResultAsset = Mantle2mDefault;
	}
	else if (MantleType == EMantleType::LowMantle)
	{
		switch (OverlayState)
		{
		case EOverlayState::Default:
		case EOverlayState::Masculine:
		case EOverlayState::Feminine:
			ResultAsset = Mantle1mDefault;
			break;
		case EOverlayState::Injured:
		case EOverlayState::Bow:
		case EOverlayState::Torch:
		case EOverlayState::Barrel:
			ResultAsset = Mantle1mLH;
			break;
		case EOverlayState::HandsTied:
			ResultAsset = Mantle1m2H;
			break;
		case EOverlayState::Rifle:
		case EOverlayState::Pistol1H:
		case EOverlayState::Pistol2H:
		case EOverlayState::Binoculars:
			ResultAsset = Mantle1mRH;
			break;
		case EOverlayState::Box:
			ResultAsset = Mantle1mBox;
			break;
		}
	}
	return ResultAsset;
}

EDrawDebugTrace::Type ACharacterBase::GetTraceDebugType(EDrawDebugTrace::Type ShowTraceType)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(PlayerController))
	{
		APlayerControllerBase* PlayerControllerBase = Cast<APlayerControllerBase>(PlayerController);
		if (IsValid(PlayerControllerBase))
		{
			ACharacter* DebugFocusCharacter = nullptr;
			bool DebugView = false;
			bool ShowHUD = false;
			bool ShowTraces = false;
			bool ShowDebugShapes = false;
			bool ShowLayerColors = false;
			bool Slomo = false;
			bool ShowCharacterInfo = false;

			PlayerControllerBase->BPIGetDebugInfo(DebugFocusCharacter, DebugView, ShowHUD, ShowTraces,
				ShowDebugShapes, ShowLayerColors, Slomo, ShowCharacterInfo);
			if (ShowTraces)
			{
				return ShowTraceType;
			}
		}
	}
	return EDrawDebugTrace::Type::None;
}

FVector ACharacterBase::GetCapsuleLocationFromBase(FVector BaseLocation, float ZOffset)
{
	if (IsValid(GetCapsuleComponent()))
	{
		float Yaw = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + ZOffset;
		return FVector(0.0f, 0.0f, Yaw);
	}
	return FVector::ZeroVector;
}

bool ACharacterBase::CapsuleHasRoomCheck(
	UCapsuleComponent* CapsuleComp,
	FVector TargetLocation,
	float HeightOffset,
	float RadiusOffset,
	EDrawDebugTrace::Type DebugTye)
{
	// 执行跟踪，查看太空舱是否有空间位于目标位置。
	if (IsValid(CapsuleComp))
	{
		float Z = CapsuleComp->GetScaledCapsuleHalfHeight_WithoutHemisphere() + RadiusOffset * -1.0f + HeightOffset;
		FVector Start = TargetLocation + FVector(0.0f, 0.0f, Z);
		FVector End = TargetLocation - FVector(0.0f, 0.0f, Z);
		float Radius = CapsuleComp->GetUnscaledCapsuleRadius() + RadiusOffset;
		FHitResult HitResult;
		TArray<AActor*> ActorsToIgnore;
		UKismetSystemLibrary::SphereTraceSingleByProfile(this, Start, End, Radius,
			FName("ALS_Character"), false, ActorsToIgnore, GetTraceDebugType(DebugTye),
			HitResult, true, FLinearColor::Green, FLinearColor::Blue, 1.0f);
		return !(HitResult.bBlockingHit || HitResult.bStartPenetrating);
	}
	return false;
}

void ACharacterBase::RagdollStart()
{
	ClearHeldObject();
	
	// Step1, 清除角色移动模式并将移动状态设置为碎布玩偶
	XXCharacterMovement->SetMovementMode(EMovementMode::MOVE_None);
	BPISetMovementState(EMovementState::Ragdoll);

	// Step2, 禁用胶囊碰撞并启用从骨盆开始的网格物理模拟。
	if (IsValid(GetCapsuleComponent()))
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		if (IsValid(GetMesh()))
		{
			GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
			GetMesh()->SetAllBodiesBelowSimulatePhysics(UKismetSystemLibrary::MakeLiteralName(FName("pelvis")), true, true);
		}
	}

	// step3, 停止任何活跃的蒙太奇。
	if (IsValid(MainAnimInstance))
	{
		MainAnimInstance->Montage_Stop(0.2f);
	}
}

void ACharacterBase::RagdollUpdate()
{
	if (!IsValid(GetMesh()))
	{
		return;
	}
	
	LastRagdollVelocity = GetMesh()->GetPhysicsLinearVelocity(FName("root"));
	
	float Spring = UKismetMathLibrary::MapRangeClamped(LastRagdollVelocity.Length(),
		0.0f, 1000.0f, 0.0f, 25000.0f);
	GetMesh()->SetAllMotorsAngularDriveParams(Spring, 0.0f, 0.0f, false);

	GetMesh()->SetEnableGravity(LastRagdollVelocity.Z > -4000.0f);

	SetActorLocationDuringRagdoll();
}

void ACharacterBase::RagdollEnd()
{
	// step1
	if (IsValid(MainAnimInstance))
	{
		MainAnimInstance->SavePoseSnapshot(FName("RagdollPose"));
	}

	// step2
	if (IsValid(XXCharacterMovement))
	{
		if (RagdollOnGround)
		{
			XXCharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking);
			MainAnimInstance->Montage_Play(GetGetUpAnimation(RagdollFaceUp));
		}
		else
		{
			XXCharacterMovement->SetMovementMode(EMovementMode::MOVE_Falling);
			XXCharacterMovement->Velocity = LastRagdollVelocity;
		}
	}

	// step3
	if (IsValid(GetCapsuleComponent()) && IsValid(GetMesh()))
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
		GetMesh()->SetAllBodiesSimulatePhysics(false);
	}

	UpdateHeldObject();
}

UAnimMontage* ACharacterBase::GetGetUpAnimation(bool bRagdollFaceUp)
{
	UAnimMontage* ResultAnimMontage = nullptr;
	switch (OverlayState)
	{
		// todo ,需要加载
	case EOverlayState::Default:
	case EOverlayState::Masculine:
	case EOverlayState::Feminine:
		ResultAnimMontage = bRagdollFaceUp ? GetUpBackDefault.Get() : GetUpFrontDefault.Get();
		break;
	case EOverlayState::Injured:
	case EOverlayState::Bow:
	case EOverlayState::Torch:
	case EOverlayState::Barrel:
		ResultAnimMontage = bRagdollFaceUp ? GetUpBackLH.Get() : GetUpFrontLH.Get();
		break;
	case EOverlayState::HandsTied:
	case EOverlayState::Box:
		ResultAnimMontage = bRagdollFaceUp ? GetUpBack2H.Get() : GetUpFront2H.Get();
		break;
	case EOverlayState::Rifle:
	case EOverlayState::Pistol1H:
	case EOverlayState::Pistol2H:
	case EOverlayState::Binoculars:
		ResultAnimMontage = bRagdollFaceUp ? GetUpBackRH.Get() : GetUpFrontRH.Get();
		break;
	}
	return ResultAnimMontage;
}

void ACharacterBase::SetActorLocationDuringRagdoll()
{
	if (!IsValid(GetMesh()) || (!IsValid(GetCapsuleComponent()) || (!IsValid(GetWorld()))))
	{
		return;
	}

	FVector TargetRagdollLocation = GetMesh()->GetSocketLocation(FName("pelvis"));
	RagdollFaceUp = TargetRagdollLocation.X < 0.0f;
	float Yaw = RagdollFaceUp ? TargetRagdollLocation.Z - 180.0f: TargetRagdollLocation.Z;
	FRotator TargetRagdollRotation = FRotator(0.0, Yaw, 0.0f);

	FVector TraceEnd = FVector(
		TargetRagdollLocation.X,
		TargetRagdollLocation.Y,
		TargetRagdollLocation.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TargetRagdollLocation, TraceEnd, ECC_Visibility);
	if (RagdollOnGround)
	{
		float NewLocationZ = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 
			FMath::Abs(HitResult.ImpactPoint.Z - HitResult.TraceStart.Z) +
			2.0f;
		FVector NewLocation = FVector(TargetRagdollLocation.X, TargetRagdollLocation.Y, NewLocationZ);

		struct FHitResult SweepHitResult;
		SetLocationAndRotation(NewLocation, TargetRagdollRotation, false, false, SweepHitResult);
	}
	else
	{
		struct FHitResult SweepHitResult;
		SetLocationAndRotation(TargetRagdollLocation, TargetRagdollRotation, false, false, SweepHitResult);
	}
}

bool ACharacterBase::SetLocationAndRotation(
	FVector NewLocation,
	FRotator NewRotation,
	bool bSweep,
	bool bTeleport,
	struct FHitResult& SweepHitResult)
{
	TargetRotation = NewRotation;
	return K2_SetActorLocationAndRotation(NewLocation, NewRotation, bSweep, SweepHitResult, bTeleport);
}

FVector ACharacterBase::GetCapsuleBaseLocation(float ZOffset)
{
	const UCapsuleComponent* CapsuleComponent = GetCapsuleComponent();
	if (IsValid(CapsuleComponent))
	{
		return CapsuleComponent->K2_GetComponentLocation() -
			CapsuleComponent->GetUpVector() * (CapsuleComponent->GetScaledCapsuleHalfHeight() + ZOffset);
	}
	return FVector::ZeroVector;
}

FVector ACharacterBase::GetPlayerMovementInput()
{
	// Todo：获取MoveForward/Backwards的输入
	float MoveForward = 0.0f;
	// Todo：获取MoveRight/Left的输入
	float MoveRight = 0.0f;
	FVector ForwardVector = FVector::ZeroVector;
	FVector RightVector = FVector::ZeroVector;
	GetControlVector(ForwardVector, RightVector);
	FVector Input = ForwardVector * MoveForward + RightVector * MoveRight;
	Input.Normalize();
	return Input;
}

// Get Control Forward/Right Vector
void ACharacterBase::GetControlVector(FVector& ForwardVector, FVector& RightVector)
{
	ForwardVector = UKismetMathLibrary::GetForwardVector(FRotator(0.0f, GetControlRotation().Yaw, 0.0f));
	RightVector = UKismetMathLibrary::GetRightVector(FRotator(0.0f, GetControlRotation().Yaw, 0.0f));
}

void ACharacterBase::UpdateGroundedRotation()
{
	if (MovementAction == EMovementAction::None)
	{
		if (CanUpdateMovingRotation())
		{
			switch (RotationMode)
			{
			case ERotationMode::VelocityDirection:
				SmoothCharacterRotation(
					FRotator(0.0f, LastVelocityRotation.Yaw, 0.0f),
					800,
					CalculateGroundedRotationRate());
				break;
			case ERotationMode::LookingDirection:
				if (Gait == EGait::Running || Gait == EGait::Walking)
				{
					float TargetYaw = GetControlRotation().Yaw + GetAnimCurveValue(FName("YawOffset"));
					SmoothCharacterRotation(
						FRotator(0, TargetYaw, 0),
						500.0f,
						CalculateGroundedRotationRate());
				}
				else if (Gait == EGait::Sprinting)
				{
					SmoothCharacterRotation(
						FRotator(0, LastVelocityRotation.Yaw, 0),
						500.0f,
						CalculateGroundedRotationRate());
				}
				break;
			case ERotationMode::Aiming:
				SmoothCharacterRotation(
						FRotator(0, GetControlRotation().Yaw, 0),
						1000.0f,
						20.0f);
			default:
				break;
			}
		}
		else
		{
			auto ApplyRotation = [this]()
			{
				float AnimCurve = GetAnimCurveValue(FName("RotationAmount"));
				if (FMath::Abs(AnimCurve) > 0.001f)
				{
					float DeltaRotationYaw = UGameplayStatics::GetWorldDeltaSeconds(this) / (1.0f / 30.0f) * AnimCurve;
					AddActorWorldRotation(FRotator(0, DeltaRotationYaw, 0));
					TargetRotation = GetActorRotation();
				}
			};
			
			if ((ViewMode == EViewMode::ThirdPerson &&
				(RotationMode == ERotationMode::VelocityDirection || RotationMode == ERotationMode::LookingDirection)))
			{
				ApplyRotation();
			}
			
			if (ViewMode == EViewMode::FirstPerson ||
				(ViewMode == EViewMode::ThirdPerson && RotationMode == ERotationMode::Aiming))
			{
				LimitRotation(-100.0f, 100.0f, 20.0f);
				ApplyRotation();
			}
		}
	}
	else if (MovementAction == EMovementAction::Rolling)
	{
		if (HasMovementInput)
		{
			SmoothCharacterRotation(FRotator(0, LastMovementInputRotation.Yaw, 0), 0.0f, 2.0f);
		}
	}
}

float ACharacterBase::GetAnimCurveValue(FName CurveName)
{
	if (IsValid(MainAnimInstance))
	{
		return MainAnimInstance->GetCurveValue(CurveName);
	}
	return 0.0f;
}

void ACharacterBase::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed)
{
	float CurrrentYaw = (GetControlRotation() - GetActorRotation()).Yaw;
	bool InRange = UKismetMathLibrary::InRange_FloatFloat(
		CurrrentYaw, AimYawMin, AimYawMax, true, true);
	if (!InRange)
	{
		float TargetYaw = CurrrentYaw > 0 ? GetControlRotation().Yaw + AimYawMin : GetControlRotation().Yaw + AimYawMax;
		SmoothCharacterRotation(FRotator(0, TargetYaw, 0), 0.0, InterpSpeed);
	}
}

void ACharacterBase::SmoothCharacterRotation(FRotator InTargetRotation, float TargetInterpSpeed, float ActorInterpSpeed)
{
	TargetRotation = UKismetMathLibrary::RInterpTo_Constant(
		TargetRotation, InTargetRotation, UGameplayStatics::GetWorldDeltaSeconds(this), TargetInterpSpeed);
	FRotator ActorRotation = UKismetMathLibrary::RInterpTo(
		GetActorRotation(), TargetRotation, UGameplayStatics::GetWorldDeltaSeconds(this), ActorInterpSpeed);
	SetActorRotation(ActorRotation);
}

float ACharacterBase::CalculateGroundedRotationRate()
{
	return CurrentMovementSettings.RotationRateCurve->GetFloatValue(GetMappedSpeed()) *
		UKismetMathLibrary::MapRangeClamped(AimYawRate, 0.0f, 300.0f, 1.0f, 3.0f);
}

EGait ACharacterBase::GetAllowedGait() const
{
	if (Stance == EStance::Standing &&
		(RotationMode == ERotationMode::LookingDirection || RotationMode == ERotationMode::VelocityDirection))
	{
		switch (DesiredGait)
		{
		case EGait::Walking:
		case EGait::Running:
			return EGait::Running;
		case EGait::Sprinting:
			return CanSprint() ? EGait::Sprinting : EGait::Running;
		default:
			break;
		}
	}

	if ((Stance == EStance::Standing && RotationMode == ERotationMode::Aiming) ||
		(Stance == EStance::Crouching))
	{
		switch (DesiredGait)
		{
		case EGait::Walking:
		case EGait::Running:
			return EGait::Running;
		case EGait::Sprinting:
			return EGait::Running;
		default:
			break;
		}
	}
	return EGait::Walking;
}

EGait ACharacterBase::GetActualGait(EGait InAllowedGait) const
{
	if (Speed >= CurrentMovementSettings.RunSpeed + 10.0f)
	{
		switch (InAllowedGait)
		{
		case EGait::Walking:
		case EGait::Running:
			return EGait::Running;
		case EGait::Sprinting:
			return EGait::Sprinting;
		default:
			break;
		}
	}
	else
	{
		if (Speed >= CurrentMovementSettings.WalkSpeed + 10)
		{
			return EGait::Running;
		}
		else
		{
			return EGait::Walking;
		}
	}
	return EGait::Walking;
}

void ACharacterBase::UpdateDynamicMovementSettings(EGait InAllowedGait)
{
	CurrentMovementSettings = GetTargetMovementSettings();
	
	float DesiredWalkSpeed = 0.0;
	switch (InAllowedGait)
	{
	case EGait::Walking:
		DesiredWalkSpeed = CurrentMovementSettings.WalkSpeed;
		break;
	case EGait::Running:
		DesiredWalkSpeed = CurrentMovementSettings.RunSpeed;
		break;
	case EGait::Sprinting:
		DesiredWalkSpeed = CurrentMovementSettings.SprintSpeed;
		break;
	}
	XXCharacterMovement->MaxWalkSpeed = DesiredWalkSpeed;
	XXCharacterMovement->MaxWalkSpeedCrouched = DesiredWalkSpeed;
	
	FVector CurveValue = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed());
	XXCharacterMovement->MaxAcceleration = CurveValue.X;
	XXCharacterMovement->BrakingDecelerationWalking = CurveValue.Y;
	XXCharacterMovement->GroundFriction = CurveValue.Z;
}

FMovementSettings ACharacterBase::GetTargetMovementSettings() const
{
	switch (RotationMode)
	{
	case ERotationMode::VelocityDirection:
		switch (Stance)
		{
		case EStance::Standing:
			return MovementData.VelocityDirection.Standing;
		case EStance::Crouching:
			return MovementData.VelocityDirection.Standing;
		default:
			break;
		}
	case ERotationMode::LookingDirection:
		switch (Stance)
		{
		case EStance::Standing:
			return MovementData.LookingDirection.Standing;
		case EStance::Crouching:
			return MovementData.LookingDirection.Standing;
		default:
			break;
		}
	case ERotationMode::Aiming:
		switch (Stance)
		{
		case EStance::Standing:
			return MovementData.LookingDirection.Standing;
		case EStance::Crouching:
			return MovementData.LookingDirection.Standing;
		default:
			break;
		}
	}
}

float ACharacterBase::GetMappedSpeed() const
{
	float ClampedWalkSpeed = UKismetMathLibrary::MapRangeClamped(
		Speed, 0.0f, CurrentMovementSettings.WalkSpeed,0.0, 1.0);
	float ClampedRunSpeed = UKismetMathLibrary::MapRangeClamped(
		Speed, CurrentMovementSettings.WalkSpeed, CurrentMovementSettings.RunSpeed,1.0, 2.0);
	float ClampedSprintSpeed = UKismetMathLibrary::MapRangeClamped(
		Speed, CurrentMovementSettings.RunSpeed, CurrentMovementSettings.SprintSpeed, 2.0, 3.0);

	float MappedSpeed = Speed > CurrentMovementSettings.WalkSpeed ? ClampedRunSpeed : ClampedWalkSpeed;
	MappedSpeed =  Speed > CurrentMovementSettings.RunSpeed ? ClampedSprintSpeed : MappedSpeed;
	return MappedSpeed;
}

bool ACharacterBase::CanUpdateMovingRotation()
{
	return ((IsMoving && HasMovementInput) || (Speed > 150.0)) && !HasAnyRootMotion(); 
}

bool ACharacterBase::CanSprint() const
{
	if (HasMovementInput)
	{
		switch (RotationMode)
		{
		case ERotationMode::VelocityDirection:
			return MovementInputAmount > 0.9f;
		case ERotationMode::LookingDirection:
			{
				FRotator Rotator = XXCharacterMovement->GetCurrentAcceleration().Rotation() - GetControlRotation();
				Rotator.Normalize();
				return MovementInputAmount > 0.9f && FMath::Abs(Rotator.Yaw) < 50;
			}
		case ERotationMode::Aiming:
			return false;
		default:
			break;
		}
	}
	return false;
}

void ACharacterBase::OnGaitChanged(EGait NewGait)
{
	PreviousActualGait = Gait;
	Gait = NewGait;
}

void ACharacterBase::OnMovementStateChanged(EMovementState NewMovementState)
{
	PreviousMovementState = MovementState;
	MovementState = NewMovementState;
	
	if (MovementState == EMovementState::InAir)
	{
		if (MovementAction == EMovementAction::None)
		{
			InAirRotation = GetActorRotation();
			if (Stance == EStance::Crouching)
			{
				UnCrouch();
			}
		}
		else if (MovementAction == EMovementAction::Rolling)
		{
			RagdollStart();
		}
	}
	else if (MovementState == EMovementState::Ragdoll)
	{
		if (PreviousMovementState == EMovementState::Mantling)
		{
			if (IsValid(MantleTimeline))
			{
				MantleTimeline->Stop();
			}
		}
	}
}

void ACharacterBase::OnMovementActionChanged(EMovementAction NewMovementAction)
{
	PreviousMovementAction = MovementAction;
	MovementAction = NewMovementAction;

	if (MovementAction == EMovementAction::Rolling)
	{
		Crouch();
	}
	
	if (PreviousMovementAction == EMovementAction::Rolling)
	{
		if (DesiredStance == EStance::Standing)
		{
			UnCrouch();
		}
		else if (DesiredStance == EStance::Crouching)
		{
			Crouch();
		}
	}
}

void ACharacterBase::BPIGetCurrentStates(
	TEnumAsByte<EMovementMode>& OutPawnMovementMode,
	EMovementState& OutMovementState,
	EMovementState& OutPrevMovementState,
	EMovementAction& OutMovementAction,
	ERotationMode& OutRotationMode,
	EGait& OutActualGait,
	EStance& OutActualStance,
	EViewMode& OutViewMode,
	EOverlayState& OutOverlayState)
{
	OutPawnMovementMode = XXCharacterMovement->MovementMode;
	OutMovementState = MovementState;
	OutPrevMovementState = PreviousMovementState;
	OutMovementAction = MovementAction;
	OutRotationMode = RotationMode;
	OutActualGait = Gait;
	OutActualStance = Stance,
	OutViewMode = ViewMode;
	OutOverlayState = OverlayState;
}

void ACharacterBase::BPIGetEssentialValues(
	FVector& OutVelocity,
	FVector& OutAcceleration,
	FVector& OutMovementInput,
	bool& OutIsMoving,
	bool& OutHasMovementInput,
	float& OutSpeed,
	float& OutMovementInputAmount,
	FRotator& OutAimingRotation,
	float& OutAimYawRate)
{
	OutVelocity = GetVelocity();
	OutAcceleration = Acceleration;
	OutMovementInput = XXCharacterMovement->GetCurrentAcceleration();
	OutIsMoving = IsMoving;
	OutHasMovementInput = HasMovementInput;
	OutSpeed = Speed;
	OutMovementInputAmount = MovementInputAmount;
	OutAimingRotation = GetControlRotation();
	OutAimYawRate = AimYawRate;
}

void ACharacterBase::BPISetMovementState(EMovementState NewMovementState)
{
	if (NewMovementState != MovementState)
	{
		OnMovementStateChanged(NewMovementState);
	}
}

void ACharacterBase::BPISetMovementAction(EMovementAction NewMovementAction)
{
	if (NewMovementAction != MovementAction)
	{
		OnMovementActionChanged(NewMovementAction);
	}
}

void ACharacterBase::BPISetRotationMode(ERotationMode NewRotationMode)
{
	if (NewRotationMode != RotationMode)
	{
		OnRotationModeChanged(NewRotationMode);
	}
}

void ACharacterBase::BPISetGait(EGait NewGait)
{
	if (NewGait != Gait)
	{
		OnGaitChanged(NewGait);
	}
}

void ACharacterBase::BPISetViewMode(EViewMode NewViewMode)
{
	if (NewViewMode != ViewMode)
	{
		OnViewModeChanged(NewViewMode);
	}
}

void ACharacterBase::BPISetOverlayState(EOverlayState NewOverlayState)
{
	if (NewOverlayState != OverlayState)
	{
		OnOverlayStateChanged(NewOverlayState);
	}
}

FVector ACharacterBase::BPIGetCameraTarget()
{
	if (IsValid(GetMesh()))
	{
		return GetMesh()->GetSocketLocation(FName("FP_Camera"));
	}
	return FVector::ZeroVector;
}

FTransform ACharacterBase::BPIGet3PPivotTarget()
{
	FTransform ResultTransform;
	if (IsValid(GetMesh()))
	{
		TArray<FVector> Locations;
		Locations.Add(GetMesh()->GetSocketLocation(FName("head")));
		Locations.Add(GetMesh()->GetSocketLocation(FName("root")));
		FVector AverageLocation = UKismetMathLibrary::GetVectorArrayAverage(Locations);
		ResultTransform = FTransform(GetActorRotation(), AverageLocation, FVector::OneVector);
	}
	return ResultTransform;
}

void ACharacterBase::BPIGet3PTraceParams(FVector& TraceOrigin, float& TraceRadius, TEnumAsByte<ETraceTypeQuery>& TraceChannel)
{
	if (IsValid(GetMesh()))
	{
		if (RightShoulder)
		{
			TraceOrigin = GetMesh()->GetSocketLocation(FName("TP_CameraTrace_R"));
		}
		else
		{
			TraceOrigin = GetMesh()->GetSocketLocation(FName("TP_CameraTrace_L"));
		}
	}
	TraceRadius = 15.0f;
	TraceChannel = ETraceTypeQuery::TraceTypeQuery1;
}

void ACharacterBase::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
	PreviousStance = Stance;
	Stance = EStance::Crouching;
}

void ACharacterBase::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	
	PreviousStance = Stance;
	Stance = EStance::Standing;
}

void ACharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (BreakFall)
	{
		// Breakfall Event
		if (IsValid(MainAnimInstance))
		{
			MainAnimInstance->Montage_Play(GetRollAnimation(), 1.35f);
		}
	}
	else
	{
		XXCharacterMovement->BrakingFrictionFactor = HasMovementInput ? 0.5f : 3.0f;
		FLatentActionInfo LatentInfo;
		UKismetSystemLibrary::RetriggerableDelay(this, 0.5f, LatentInfo);
		// todo, 上面的函数完成后才执行下面的内容
		XXCharacterMovement->BrakingFrictionFactor = 0.0f;
	}
}

void ACharacterBase::OnJumped_Implementation()
{
	InAirRotation = Speed > 100.0f ? LastVelocityRotation : GetActorRotation();
	if (IsValid(MainAnimInstance))
	{
		MainAnimInstance->BPIJumped();
	}
}

UAnimMontage* ACharacterBase::GetRollAnimation()
{
	UAnimMontage* ResultAnimMontage = nullptr;
	switch (OverlayState)
	{
	// todo ,需要加载
	case EOverlayState::Default:
	case EOverlayState::Masculine:
	case EOverlayState::Feminine:
		ResultAnimMontage = LandRollDefault.Get();
	break;
	case EOverlayState::Injured:
	case EOverlayState::Bow:
	case EOverlayState::Torch:
	case EOverlayState::Barrel:
		ResultAnimMontage = LandRollLH.Get();
		break;
	case EOverlayState::HandsTied:
	case EOverlayState::Box:
		ResultAnimMontage = LandRollRH.Get();
		break;
	case EOverlayState::Rifle:
	case EOverlayState::Pistol1H:
	case EOverlayState::Pistol2H:
	case EOverlayState::Binoculars:
		ResultAnimMontage = LandRollRH.Get();
		break;
	}
	return ResultAnimMontage;
}

void ACharacterBase::RollEvent()
{
	if (IsValid(MainAnimInstance))
	{
		MainAnimInstance->Montage_Play(GetRollAnimation(), 1.15f);
	}
}