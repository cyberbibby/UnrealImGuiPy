// Copyright (c) 2024 Bibby. All Rights Reserved.

#include "NetImguiEditorToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FNetImguiEditorToolbarModule"

void FNetImguiEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(LaunchNetImguiServerAction, "NetImgui Server", "Launch a NetImgui Server", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
