// Copyright (c) 2024 Bibby. All Rights Reserved.


#include "NetImguiEditorToolbarStyle.h"

#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FNetImguiEditorToolbarStyle::StyleInstance = NULL;

void FNetImguiEditorToolbarStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FNetImguiEditorToolbarStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FNetImguiEditorToolbarStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("NetImguiEditorToolbarStyle"));
	return StyleSetName;
}

namespace
{
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
} // namespace

TSharedRef<FSlateStyleSet> FNetImguiEditorToolbarStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("NetImguiEditorToolbarStyle"));

	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ImGui")->GetBaseDir() / TEXT("Resources"));

	Style->Set("NetImguiEditorToolbar.LaunchNetImguiServerAction", new IMAGE_BRUSH(TEXT("NetImguiServer"), Icon40x40));
	Style->Set("NetImguiEditorToolbar.LaunchNetImguiServerAction.Small", new IMAGE_BRUSH(TEXT("NetImguiServer@0.5x"), Icon20x20));

	return Style;
}

#undef IMAGE_BRUSH

void FNetImguiEditorToolbarStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FNetImguiEditorToolbarStyle::Get()
{
	return *StyleInstance;
}
