// Copyright (c) 2024 Bibby. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FNetImguiEditorToolbarModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
	void AddToolbarExtension();

	void OnLaunchNetImguiServerButtonClicked();
	bool CanExecuteLaunchNetImguiServer();

	TSharedPtr<FUICommandList> NetImguiCommands;
	uint32 OutProcessId = 0;
	FProcHandle NetImguiServerProcessHandle;
};
