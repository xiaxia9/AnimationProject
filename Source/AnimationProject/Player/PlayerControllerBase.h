// Copyright XiaWen, Inc. All Rights Reserved.

#pragma once
#include "AnimationProject/Common/CommonInterfaces.h"

#include "PlayerControllerBase.generated.h"

UCLASS()
class APlayerControllerBase : public APlayerController, public IControllerInterface
{
	GENERATED_BODY()
public:
	APlayerControllerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	// interface begin......
	virtual void BPIGetDebugInfo(ACharacter* DebugFocusCharacter,
					bool& DebugView,
					bool& ShowHUD,
					bool& ShowTraces,
					bool& ShowDebugShapes,
					bool& ShowLayerColors,
					bool& Slomo,
					bool& ShowCharacterInfo) override;
};

