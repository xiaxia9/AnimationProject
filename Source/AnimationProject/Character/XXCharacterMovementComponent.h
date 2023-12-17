// Copyright Xia Wen, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "XXCharacterMovementComponent.generated.h"


UCLASS(Config = Game)
class ANIMATIONPROJECT_API UXXCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UXXCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
};