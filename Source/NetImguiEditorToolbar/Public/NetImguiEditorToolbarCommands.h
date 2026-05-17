// Copyright (c) 2024 Bibby. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

#include "NetImguiEditorToolbarStyle.h"

class FNetImguiEditorToolbarCommands : public TCommands<FNetImguiEditorToolbarCommands>
{
public:
	FNetImguiEditorToolbarCommands()
			: TCommands<FNetImguiEditorToolbarCommands>(TEXT("NetImguiEditorToolbar"), NSLOCTEXT("Contexts", "NetImguiEditorToolbar", "NetImgui Editor Toolbar"), NAME_None, FNetImguiEditorToolbarStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> LaunchNetImguiServerAction;
};
