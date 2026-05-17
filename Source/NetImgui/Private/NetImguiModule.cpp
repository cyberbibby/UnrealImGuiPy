// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "NetImguiModule.h"

#include <CoreMinimal.h>
#include <Misc/App.h>
#include <Misc/CoreDelegates.h>
#include <HAL/FileManager.h>
#include <HAL/IConsoleManager.h>

#if NETIMGUI_ENABLED
#include "NetImgui_Api.h"

#include "ImGuiContextProxy.h"
#include <Interfaces/IPluginManager.h>
#include <SocketSubsystem.h>
#include "IconsFontAwesome6.h"
#include "IconsMaterialDesign.h"

// List of defines to easily use Icons available in 'Kenney's Game Icons'
// For list available icons, see: https://kenney.nl/assets/game-icons and https://kenney.nl/assets/game-icons-expansion
#include "IconsKenney.h"
#include "FontKenney/KenneyIcon.cpp"


//=================================================================================================
// FontCreationCallback
//-------------------------------------------------------------------------------------------------
//
//=================================================================================================
void FontCreationCallback(float PreviousDPIScale, float NewDPIScale)
{
	IM_UNUSED(PreviousDPIScale);
	IM_UNUSED(NewDPIScale);
	FNetImguiModule::Get().BuildFont(NewDPIScale);
}

//=================================================================================================
// If engine was launched with "-netimguiserver [hostname]", try connecting directly
// to the NetImguiServer application at the provided address.
//
// [hostname] can be an ip address, a window pc hostname, etc...
//
// You can also change the default port by appending ":[port number]"
// The default NetImguiServer port is 8888 (unless modified in NetImgui.Build.cs)
//
//-------------------------------------------------------------------------------------------------
// COMMANDLINE EXAMPLES
// "-NetImguiConnect"					Try connecting to NetImguiServer at 'localhost : (default port)'
// "-NetImguiConnect localhost"			Try connecting to NetImguiServer at 'localhost : (default port)'
// "-NetImguiConnect 192.168.1.2"		Try connecting to NetImguiServer at '192.168.1.2 : (default port)'
// "-NetImguiConnect 192.168.1.2:60"	Try connecting to NetImguiServer at '192.168.1.2 : 60'
//=================================================================================================
void FNetImguiModule::TryConnectingToServer(const FString& HostnameAndPort)
{
	checkfSlow(IsInGameThread(), TEXT("TryConnectingToServer() should be called from the GameThread"));

#if PLATFORM_IOS
	FString sessionName = FString::Format(TEXT("{0}-{1}"), {FApp::GetProjectName(), *FPlatformMisc::GetDeviceId().Left(8)});
#else
	FString sessionName = FString::Format(TEXT("{0}-{1}"), {FApp::GetProjectName(), FPlatformProcess::ComputerName()});
#endif
	FString hostName = "localhost";
	int32_t hostPort = NETIMGUI_CONNECTPORT;

	if (!HostnameAndPort.IsEmpty())
	{
		int pos = HostnameAndPort.Find(TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		hostName = HostnameAndPort;
		if (pos > 0)
		{
			FString portNumber = HostnameAndPort.Right(HostnameAndPort.Len() - pos - 1);
			hostName.LeftInline(pos);
			hostPort = FCString::Atoi(*portNumber);
			hostPort = (hostPort == 0) ? NETIMGUI_CONNECTPORT : hostPort; // Restore Port Number if integer conversion failed
		}
	}

	ImGui::SetCurrentContext(NetImguiContextPtr);

	if (NetImgui::ConnectToApp(TCHAR_TO_ANSI(sessionName.GetCharArray().GetData()), TCHAR_TO_ANSI(*hostName), hostPort, nullptr, FontCreationCallback))
	{
		checkf(NetImguiContextPtr == NetImgui::GetContext(), TEXT("NetImguiContextPtr should be the same as NetImgui::GetContext()"));
	}
}

//=================================================================================================
// If the plugin was not able to reach the NetImguiServer (not requested, or bad address),
// starts waiting for a connection from the NetImguiServer instead.
//
// Will start waiting for a connection on a default port id (based on engine type)
// (unless modified in NetImgui.Build.cs)
// NETIMGUI_LISTENPORT_GAME				= 8889
// NETIMGUI_LISTENPORT_EDITOR			= 8890
// NETIMGUI_LISTENPORT_DEDICATED_SERVER	= 8891
//
// User can request a specific port by using commandline option "-netimguiport [Port Number]"
//
//-------------------------------------------------------------------------------------------------
// COMMANDLINE EXAMPLES
// (empty)					Game will waits for connection on Default Port
// "-NetImguiListen"		Game will waits for connection on Default Port
// "-NetImguiListen 10000"	Game will waits for connection on Port '10000'
//=================================================================================================
void FNetImguiModule::TryListeningForServer(const FString& ListeningPort)
{
	checkfSlow(IsInGameThread(), TEXT("TryListeningForServer() should be called from the GameThread"));

	if (!NetImgui::IsConnectionPending() && !NetImgui::IsConnected())
	{
		FString sessionName = FString::Format(TEXT("{0}-{1}"), {FApp::GetProjectName(), FPlatformProcess::ComputerName()});
		uint32_t listenPort = FCString::Atoi(*ListeningPort);
		if (listenPort == 0)
		{
			listenPort = IsRunningDedicatedServer()
				             ? NETIMGUI_LISTENPORT_DEDICATED_SERVER
				             : FApp::IsGame()
				             ? NETIMGUI_LISTENPORT_GAME
				             : NETIMGUI_LISTENPORT_EDITOR;
		}

		ImGui::SetCurrentContext(NetImguiContextPtr);

		if (NetImgui::ConnectFromApp(TCHAR_TO_ANSI(sessionName.GetCharArray().GetData()), listenPort, nullptr, FontCreationCallback))
		{
			checkf(NetImguiContextPtr == NetImgui::GetContext(), TEXT("NetImguiContextPtr should be the same as NetImgui::GetContext()"));
		}
	}
}

static void CommandConnect(const TArray<FString>& Args)
{
	FNetImguiModule::Get().TryConnectingToServer(Args.Num() > 0 ? Args[0] : "");
}

static void CommandListen(const TArray<FString>& Args)
{
	FNetImguiModule::Get().TryListeningForServer(Args.Num() > 0 ? Args[0] : "");
}

static void CommandDisconnect(const TArray<FString>& Args)
{
	NetImgui::Disconnect();
}

static FAutoConsoleCommand GNetImguiConnectCmd
(
	TEXT("NetImguiConnect"),
	TEXT("Try connecting to the NetImgui Remoter server.\nNetImguiConnect [hostname/ip]:[Port]\n(Connect to localhost by default)"),
	FConsoleCommandWithArgsDelegate::CreateStatic(CommandConnect)
);

static FAutoConsoleCommand GNetImguiListenCmd
(
	TEXT("NetImguiListen"),
	TEXT("Start listening for a connection from the NetImgui Remote Server.\nNetImguiListen [Port]\n(Use default port when not specified)"),
	FConsoleCommandWithArgsDelegate::CreateStatic(CommandListen)
);

static FAutoConsoleCommand GNetImguiDisconnectCmd
(
	TEXT("NetImguiDisconnect"),
	TEXT("Stop any connection with the NetImgui Server and also stop listening for one."),
	FConsoleCommandWithArgsDelegate::CreateStatic(CommandDisconnect)
);

//=================================================================================================
// Update
//-------------------------------------------------------------------------------------------------
// Main update method of this plugin.
//	1. Finish the previous Dear Imgui frame drawing
//	2. Start a new Dear ImGui drawing frame (when needed)
//	3. Call all listeners to draw their Dear Imgui content
//
// Dear ImGui content can be drawn anywhere in your own code (on the gamethread) as long as
// 'NetImguiHelper::IsDrawing()' is true. You can also add a listener to 'FNetImguiModule::OnDrawImgui'
// and draw inside the callback (without need to check for 'NetImguiHelper::IsDrawing()')
//
// For examples on how to use UnrealNetImgui with Dear ImGui, take a look at :
//		'Plugins\NetImgui\Source\Sample\NetImguiDemoActor.cpp'
//=================================================================================================
// void FNetImguiModule::Update()
// {
// 	if (NetImgui::IsDrawing())
// 		NetImgui::EndFrame();
//
// #if NETIMGUI_FRAMESKIP_ENABLED //Not interested in drawing Dear ImGui Content, until connection established
// 	if (NetImgui::IsConnected())
// #endif
// 	{
// 		NetImgui::NewFrame(NETIMGUI_FRAMESKIP_ENABLED);
// 		if (NetImgui::IsDrawingRemote())
// 		{
// 			//----------------------------------------------------------------------------
// 			// Ask all listener to draw their Dear ImGui content
// 			//----------------------------------------------------------------------------
// 			OnDrawImgui.Broadcast();
// 		}
// 	}
// }

//=================================================================================================
// BuildFont
//-------------------------------------------------------------------------------------------------
// Regenerate the Font Atlas when not already created or the DPI scaled changed
//=================================================================================================
void FNetImguiModule::BuildFont(float InFontDPIScale)
{
	ImGuiContext* SavedContext = ImGui::GetCurrentContext();
	ImGui::SetCurrentContext(NetImguiContextPtr);
	ImFontAtlas* FontAtlas = ImGui::GetIO().Fonts;
	bool bNeedBuild = FontDPIScale <= 0.f || !FontAtlas->IsBuilt();

	// Detect if the new DPI change the pixel size of any of our font
	for (int i = 0; !bNeedBuild && i < FontAtlas->Fonts.size(); ++i)
	{
		int PixelSizeNative = static_cast<int>(FontAtlas->Fonts[i]->LegacySize / FontDPIScale);
		int PixelSizeNeeded = static_cast<int>(static_cast<float>(PixelSizeNative) * InFontDPIScale);
		bNeedBuild = PixelSizeNeeded != static_cast<int>(FontAtlas->Fonts[i]->LegacySize);
	}

	// We need to generate the font, proceed with its creation/update
	if (bNeedBuild)
	{
		FontAtlas->Clear(); // Clear previous font data, otherwise it will be duplicated.

		FontDPIScale = InFontDPIScale;
		ImFontConfig FontConfig;
		FontConfig.FontDataOwnedByAtlas = false;
		FontConfig.SizePixels = 13.f * FontDPIScale;

		FontAtlas->AddFontDefault(&FontConfig); // Proggy Clean

		ImFont* DefaultFont = FontAtlas->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
			TCHAR_TO_ANSI(*FPaths::Combine(FPaths::EngineContentDir(), TEXT("Slate"), TEXT("Fonts"), TEXT("Roboto-Regular.ttf"))),
#else
			TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
				*FPaths::Combine(FPaths::EngineContentDir(), TEXT("Slate"), TEXT("Fonts"), TEXT("Roboto-Regular.ttf")))),
#endif
			FMath::RoundToFloat(16.f * FontDPIScale), &FontConfig, FontAtlas->GetGlyphRangesDefault());

		FontConfig.MergeMode = true;
		FontConfig.PixelSnapH = true;

		FontAtlas->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
			TCHAR_TO_ANSI(*FPaths::Combine(FPaths::EngineContentDir(), TEXT("Slate"), TEXT("Fonts"), TEXT("DroidSansFallback.ttf"))),
#else
			TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
				*FPaths::Combine(FPaths::EngineContentDir(), TEXT("Slate"), TEXT("Fonts"), TEXT("DroidSansFallback.ttf")))),
#endif
			FMath::RoundToFloat(16.f * FontDPIScale), &FontConfig, FontAtlas->GetGlyphRangesChineseFull());

		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ImGui"));
		constexpr ImWchar IconRanges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
		FontConfig.GlyphOffset.y = 3.f;
		FontConfig.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
		FontAtlas->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
			TCHAR_TO_ANSI(*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT(FONT_ICON_FILE_NAME_FAS))),
#else
			TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
				*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT(FONT_ICON_FILE_NAME_FAS)))),
#endif
			FMath::RoundToFloat(16.f * FontDPIScale), &FontConfig, IconRanges); // Font Awesome 6 Icons

		constexpr ImWchar MatIconRanges[] = {ICON_MIN_MD, ICON_MAX_16_MD, 0};
		FontConfig.GlyphOffset.y = 5.f;
		FontAtlas->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
			TCHAR_TO_ANSI(*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT(FONT_ICON_FILE_NAME_MD))),
#else
			TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
				*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT(FONT_ICON_FILE_NAME_MD)))),
#endif
			FMath::RoundToFloat(16.f * FontDPIScale), &FontConfig, MatIconRanges); // Google's Material Design icons

		constexpr ImWchar KenneyIconRanges[] = {ICON_MIN_KI, ICON_MAX_KI, 0};
		FontConfig.GlyphOffset.y = 3.f;
		FontAtlas->AddFontFromMemoryCompressedTTF(KenneyIcon_compressed_data, KenneyIcon_compressed_size,
		                                          FMath::RoundToFloat(16.f * FontDPIScale), &FontConfig, KenneyIconRanges);

		FontAtlas->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
			TCHAR_TO_ANSI(*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT("JetBrainsMonoNL-Medium.ttf"))),
#else
			TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
				*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT("JetBrainsMonoNL-Medium.ttf")))),
#endif
			18.0f);

		FontAtlas->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
		// FontAtlas->TexDesiredWidth = 8 * 1024;
		FontAtlas->Build();
		FontAtlas->SetTexID(ImGuiInterops::ToImTextureID(1)); // 必须指定跟ImGui模块中FontAtlas的TexID一致，否则会出现字体显示不正确的问题（白色方块）
		ImGui::GetIO().FontDefault = DefaultFont;

		uint8_t* PixelData(nullptr);
		int Width(0), Height(0);
		FontAtlas->GetTexDataAsAlpha8(&PixelData, &Width, &Height);
		NetImgui::SendDataTexture(FontAtlas->TexRef.GetTexID(), PixelData, static_cast<uint16_t>(Width), static_cast<uint16_t>(Height), NetImgui::eTexFormat::kTexFmtA8);
		FontAtlas->ClearTexData(); // Note: Free unneeded client texture memory. Various font size with japanese and icons can increase memory substancially(~64MB)
	}

	ImGui::SetCurrentContext(SavedContext);
}

//=================================================================================================
// IsConnected
//-------------------------------------------------------------------------------------------------
//
//=================================================================================================
FORCEINLINE bool FNetImguiModule::IsConnected() const
{
	return NetImgui::IsConnected();
}

void FNetImguiModule::NewFrame()
{
	ImGui::SetCurrentContext(NetImguiContextPtr);

	// It is possible to avoid drawing ImGui when connected and server doesn't expect a new frame,
	// but requires to skip calling drawing delegates and user not to draw in UObject::Tick.
	// Last point difficult to control, so might be safer to not support 'frameskip'
	bIsDrawing = NetImgui::NewFrame(true);

	// Only draw in the NetImguiServer.
	if (NetImgui::IsDrawingRemote())
	{
		static bool bShowHelp = false;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("ImGui"))
			{
				if (ImGui::BeginMenu("NetImgui"))
				{
					ImGui::MenuItem("Help", nullptr, &bShowHelp);

					// Add other netImgui options here, like continue display locally
					ImGui::Separator();
					if (ImGui::RadioButton("DualDisplay: Off", DualUIType == EDualUI::DualDisplay_Off)) { DualUIType = EDualUI::DualDisplay_Off; }
					if (ImGui::RadioButton("DualDisplay: On", DualUIType == EDualUI::DualDisplay_On)) { DualUIType = EDualUI::DualDisplay_On; }
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Display different Dear ImGui content in Game and NetImgui server simultaneously.\n"
							"Your Imgui code must support multi-context callback to work with this.\n"
							"注意！！由于在移动设备上，Slate的VertexBufferSize有限，如果此时Server上的界面内容过多，双显可能会导致移动设备crash");
					ImGui::EndMenu();
				}
				ImGui::Separator();
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (bShowHelp)
		{
			static const ImVec4 kColorHighlight = ImVec4(0.1f, 0.85f, 0.1f, 1.0f);
			ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("NetImgui: Help", &bShowHelp))
			{
				ImGui::TextColored(kColorHighlight, "Version :");
				ImGui::SameLine();
				ImGui::TextUnformatted("NetImgui : " NETIMGUI_VERSION);

				bool bCanBindAll = false;
				const TSharedPtr<FInternetAddr> LocalIpAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
				ImGui::NewLine();
				ImGui::TextColored(kColorHighlight, "Local IP :");
				ImGui::SameLine();
				ImGui::TextUnformatted(LocalIpAddr.IsValid() ? TCHAR_TO_ANSI(*LocalIpAddr->ToString(false)) : "Unknown");

				ImGui::NewLine();
				ImGui::TextColored(kColorHighlight, "Low 'Dear ImGui' menus responsiveness when used with editor");
				ImGui::TextWrapped("Cause   : UE editor option to 'lower CPU cost when editor is unfocused' active");
				ImGui::TextWrapped("Solution: Disable this option. (Edit->Editor Preferences->General->Performance->Use Less CPU when in background)");

				ImGui::NewLine();
				ImGui::TextColored(kColorHighlight, "Usage example");
				ImGui::TextWrapped("Samples for using Dear ImGui with the NetImgui plugin, can be found in 'UnrealNetImgui/Source/Sample/NetImguiDemoActor.cpp'.  "
					"This Demo actor isn't enable in the build. To active it, set 'bDemoActor_Enabled' to true in 'NetImgui.Build.cs'"
					"Demo Actor source code is also is also a good demonstration of how to integrate the plugin to your project");
			}
			ImGui::End();
		}
	}
}

void FNetImguiModule::EndFrame(FImGuiContextProxy* ContextProxyPtr)
{
	if (IsDrawing())
	{
		NetImgui::EndFrame();
		bIsDrawing = false;

		if (ContextProxyPtr != nullptr)
		{
			if (DualUIType == EDualUI::DualDisplay_On)
			{
				ImDrawData* pDrawData = ImGui::GetDrawData();
				// Because UpdateDrawData take ownership of the Imgui DrawData with a memory swap,
				// we make sure we only update the display when we have new draw data in, instead of
				// relying on NetImgui DrawData that got stolen by the ProxyContext
				if (pDrawData && pDrawData->CmdListsCount > 0)
				{
					ContextProxyPtr->UpdateDrawData(pDrawData);
				}
			}
			// Clear the local display
			else if (DualUIType == EDualUI::DualDisplay_Off)
			{
				ContextProxyPtr->UpdateDrawData(nullptr);
			}
		}
	}
}

bool FNetImguiModule::SetupDrawAtRemote()
{
#if NETIMGUI_ENABLED
	ImGui::SetCurrentContext(NetImguiContextPtr);
	return NetImgui::IsDrawingRemote();
#else
	return false;
#endif
}

void FNetImguiModule::SendDataTexture(ImTextureID textureId, void* pData, uint16_t width, uint16_t height, NetImgui::eTexFormat format)
{
	NetImgui::SendDataTexture(textureId, pData, width, height, format);
}

#endif // NETIMGUI_ENABLED

//=================================================================================================
// StartupModule
//-------------------------------------------------------------------------------------------------
// Initialize this module, the NetImgui client code, and Dear ImGui
//
// Note:	By default, will wait for a connection from the NetImguiServer on the port associated
//			to the engine type (editor/game/server). You can change the listening port to your
//			desired value, by editing 'NetImgui.Build.cs', or changing the function
//			'TryListeningForServer()' or using a commandline parameter. You can also directly
//			try connecting to the server by using another commandline parameter. Please take a
//			look at 'TryConnectingToServer()' / 'TryListeningForServer()' for more informations.
//=================================================================================================
void FNetImguiModule::StartupModule()
{
#if NETIMGUI_ENABLED
	NetImgui::Startup();

	NetImguiContextPtr = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	BuildFont(1.f);
#if PLATFORM_WINDOWS
	//---------------------------------------------------------------------------------------------
	// Setup connection to wait for netImgui server to reach us
	// Note:	The default behaviour is for the Game Client to wait for connection from the NetImgui Server
	//---------------------------------------------------------------------------------------------
	// Commandline request for a connectino to NetImguiServer
	FString HostnameAndPort, ListeningPort;
	if (FParse::Value(FCommandLine::Get(), TEXT("NetImguiConnect"), HostnameAndPort))
	{
		TryConnectingToServer(HostnameAndPort);
	}

	// If failed connecting, start listening for the NetImguiServer
	if (FParse::Value(FCommandLine::Get(), TEXT("NetImguiListen"), ListeningPort) || NETIMGUI_WAITCONNECTION_AUTO_ENABLED)
	{
		TryListeningForServer(ListeningPort);
	}
#endif //PLATFORM_WINDOWS
#endif //NETIMGUI_ENABLED
}

//=================================================================================================
// Shutdown
//-------------------------------------------------------------------------------------------------
// Free up resources when module is unloaded
//=================================================================================================
void FNetImguiModule::ShutdownModule()
{
#if NETIMGUI_ENABLED
	FCoreDelegates::OnEndFrame.Remove(OnCheckCallback);
	OnCheckCallback.Reset();

	EndFrame(nullptr);

	NetImgui::Shutdown();

	if (NetImguiContextPtr != nullptr)
	{
		ImGui::DestroyContext(NetImguiContextPtr);
		NetImguiContextPtr = nullptr;
	}
#endif //NETIMGUI_ENABLED
}

//=================================================================================================
// For convenience and easy access to the netImgui source code, we build it as part of this module.
//=================================================================================================
#if NETIMGUI_ENABLED

#include <NetImgui_Api.cpp>
#include <NetImgui_Client.cpp>
#include <NetImgui_CmdPackets_DrawFrame.cpp>
#include <NetImgui_NetworkUE4.cpp>

#endif // NETIMGUI_ENABLED

#define LOCTEXT_NAMESPACE "FNetImguiModule"
IMPLEMENT_MODULE(FNetImguiModule, NetImgui)
#undef LOCTEXT_NAMESPACE
