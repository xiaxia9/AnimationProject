// Copyright 2022-2023 XiaWen. All Rights Reserved.

#include "PhysicsAnimationToolsStyle.h"
#include "PhysicsAnimationToolsEditor.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FPhysicsAnimationToolsStyle::StyleInstance = NULL;

void FPhysicsAnimationToolsStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		// StyleInstance = Create();
		// FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FPhysicsAnimationToolsStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FPhysicsAnimationToolsStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("PhysicsAnimationStyle"));
	return StyleSetName;
}

TSharedPtr<class ISlateStyle> FPhysicsAnimationToolsStyle::Get()
{
	return StyleInstance;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT