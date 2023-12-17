#include "PlayerControllerBase.h"

APlayerControllerBase::APlayerControllerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void APlayerControllerBase::BPIGetDebugInfo(ACharacter* DebugFocusCharacter,
		bool& DebugView,
		bool& ShowHUD,
		bool& ShowTraces,
		bool& ShowDebugShapes,
		bool& ShowLayerColors,
		bool& Slomo,
		bool& ShowCharacterInfo)
{
	// todo1
}