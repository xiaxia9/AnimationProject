// Copyright 2022-2023 XiaWen. All Rights Reserved.

#pragma once
#include "SlateBasics.h"

class FPhysicsAnimationToolsStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static FName GetStyleSetName();
	static TSharedPtr<class ISlateStyle> Get();

private:
	static TSharedPtr<class FSlateStyleSet> StyleInstance;

};