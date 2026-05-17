// Copyright (c) 2024 Bibby. All Rights Reserved.

#include "NetImguiEditorToolbar.h"

#include "LevelEditor.h"
#include "ToolMenus.h"

#include "NetImguiEditorToolbarStyle.h"
#include "NetImguiEditorToolbarCommands.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FNetImguiEditorToolbarModule"

void FNetImguiEditorToolbarModule::StartupModule()
{
	FNetImguiEditorToolbarStyle::Initialize();
	FNetImguiEditorToolbarStyle::ReloadTextures();
	FNetImguiEditorToolbarCommands::Register();

	NetImguiCommands = MakeShareable(new FUICommandList);

	NetImguiCommands->MapAction(
		FNetImguiEditorToolbarCommands::Get().LaunchNetImguiServerAction,
		FExecuteAction::CreateRaw(this, &FNetImguiEditorToolbarModule::OnLaunchNetImguiServerButtonClicked),
		FCanExecuteAction::CreateRaw(this, &FNetImguiEditorToolbarModule::CanExecuteLaunchNetImguiServer));

	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(NetImguiCommands.ToSharedRef());

	// Inject the button
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FNetImguiEditorToolbarModule::AddToolbarExtension));
}

void FNetImguiEditorToolbarModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (NetImguiServerProcessHandle.IsValid() && FPlatformProcess::IsProcRunning(NetImguiServerProcessHandle))
	{
		FPlatformProcess::TerminateProc(NetImguiServerProcessHandle); // 让NetImgui Server进程与Editor进程共生
	}

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FNetImguiEditorToolbarStyle::Shutdown();
	FNetImguiEditorToolbarCommands::Unregister();
}

void FNetImguiEditorToolbarModule::AddToolbarExtension()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FNetImguiEditorToolbarCommands::Get().LaunchNetImguiServerAction));
				Entry.SetCommandList(NetImguiCommands);
				Entry.Name = "LaunchNetImguiServerButton";
			}
		}
	}
}

void FNetImguiEditorToolbarModule::OnLaunchNetImguiServerButtonClicked()
{
	FString OutResults, OutErrors;
	FString NetImguiServerPath = IPluginManager::Get().FindPlugin("ImGui")->GetBaseDir() / TEXT("Tools") / TEXT("NetImguiServer");
	FString NetImguiServerURL = NetImguiServerPath / TEXT("NetImguiServer.exe");

	NetImguiServerProcessHandle = FPlatformProcess::CreateProc(*NetImguiServerURL, NULL, false, false, false, &OutProcessId, 0, *NetImguiServerPath, NULL, NULL);
}

bool FNetImguiEditorToolbarModule::CanExecuteLaunchNetImguiServer()
{
#if PLATFORM_WINDOWS
	return !NetImguiServerProcessHandle.IsValid() || !FPlatformProcess::IsProcRunning(NetImguiServerProcessHandle); // TODO: 检测是否有相同进程存在，只允许开启一个NetImgui Server应用？
#else
	return false;
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNetImguiEditorToolbarModule, NetImguiEditorToolbar)
