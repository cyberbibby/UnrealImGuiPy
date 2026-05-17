// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

//=================================================================================================
// NetImgui Plugin Build Setup
//-------------------------------------------------------------------------------------------------
// Plugin exposing Dear ImGui library for drawing 2D menus. These menus are displayed and
// controlled from an external application ("Plugin\UnrealNetImgui\NetImguiServer\netImguiServer.exe")
// but processed from this engine code. Works on various platform supported by UE4 and UE5
//
// Note:	Displaying Dear ImGui menus InGame, can be done by using the plugin UnrealImgui instead
//			(https://github.com/segross/UnrealImGui). It was designed for local display of
//			Imgui content but also has netImgui support in 'net_imgui' branch
//
//-------------------------------------------------------------------------------------------------
// USAGE
// Enable this plugin, start the NetImguiServer.exe application included with this plugin, and
// start your game/editor. A connection should be established automatically when on the same PC.
//
// To connect on remote game, you can launch the game with a commandline parameter
// to request a connection a running instance of NetImguiServer :
//	'-netimguiserver [NETIMGUISERVER PC HOSTNAME/IP]'
//
// or add the Client Hostname/IP in the NetImguiServer configs, to let the remote app try to connect
// to the game instead. Note: You can specify a custom port for this client to wait connection on
//  '-netimguiport [PORT NUMBER]
//
// Note:	If a connection to NetImguiServer was requested but failed, will default to waiting
//			for a connection from the NetImguiServer instead.
//
//-------------------------------------------------------------------------------------------------
// COMMANDLINE EXAMPLES
//-------------------------------------------------------------------------------------------------
// (empty)							Game will waits for connection on Port 'NETIMGUI_LISTENPORT_GAME / _EDITOR / _DEDICATED_SERVER (based on engine type)
// "-netimguiport 10000"			Game will waits for connection on Port '10000'
// "-netimguiserver"				Try connecting to NetImguiServer at 'localhost : NETIMGUI_CONNECTPORT'
// "-netimguiserver localhost"		Try connecting to NetImguiServer at 'localhost : NETIMGUI_CONNECTPORT'
// "-netimguiserver 192.168.1.2"	Try connecting to NetImguiServer at '192.168.1.2 : NETIMGUI_CONNECTPORT'
// "-netimguiserver 192.168.1.2:60"	Try connecting to NetImguiServer at '192.168.1.2 : 60'
//
//-------------------------------------------------------------------------------------------------
// Dear ImGui Library	: v1.90.1	docking (https://github.com/ocornut/imgui)
// NetImGui Library		: v1.10	(https://github.com/sammyfreg/netImgui)
// Tested on Unreal Engine 5.3
//=================================================================================================

public class NetImgui : ModuleRules
{
	public NetImgui(ReadOnlyTargetRules Target) : base(Target)
	{
		//=========================================================================================
		// User Configuration: Basic settings
		//=========================================================================================

		// Toggle NetImgui support here
		bool bNetImgui_Enabled = true;
		//---------------------------------------------------------------------

		// When true, only redraw Dear ImGui when needed, saving processing.
		// When enabled, user must check "NetImguiHelper::IsDrawing()" before emiting ImGui draws
		bool bFrameSkip_Enabled = true;
		//---------------------------------------------------------------------

		// When true, the plugin will automatically start listening for a connection from the NetImguiServer
		// You can disable it, and rely on launching the game with netimgui commandline options
		// or using a UnrealCommand to connect/start listening
		bool bAutoWaitConnection_Enabled = true;
		//---------------------------------------------------------------------

		// When true, use the 'FreeType' library to generate the font texture
		// This means including the Freetype library (already included with editor) in the build
		// Generates sligthly better result than the default stb_truetype default code
		bool bFreeType_Enabled = false;
		//---------------------------------------------------------------------

		// When true, the Dear ImGui demo window will be available in the NetImgui mainmenu bar.
		// Usefull as a reference on what programmer can do with Dear ImGui
		bool bDemoImgui_Enabled = false;
		//---------------------------------------------------------------------

		// When true, the demo actor 'ANetImguiDemoActor' will be available to use in your game.
		// Can be found in 'NetImguiDemoActor.cpp', demonstrating how to use NetImgui in your own project
		bool bDemoActor_Enabled = false;

		//=========================================================================================
		// User Configuration: Network
		//=========================================================================================

		// Com Port used by this client, to try connecting to the remote NetImgui Server (8888 by default)
		// Used when engine is launched with command line parameter 'netimguiserver' to request a connection
		// attempt, instead of waiting for server to reach the game
		string kRemoteConnectPort = "(NetImgui::kDefaultServerPort)";
		//---------------------------------------------------------------------

		// Com Port used by Game exe to wait for a connection from netImgui Server (8889 by default)
		// NetImgui Server will try to find running game client on this port and connect to them
		// Note:	Server will find client running on same PC, on this port
		//			To find client on remote connection (running on console, smartphone, other PC, ...)
		//			you will need to add their IP in the Server Client list.
		//			Alternatively, you can modify the connection code in 'FNetImguiModule::StartupModule()'
		//			to let the client connect directly to NetImGui server using 'NetImgui::ConnectToApp(ServerIP)'
		string kGameListenPort = "(NetImgui::kDefaultClientPort)";
		//---------------------------------------------------------------------

		// Com Port used by Editor exe to wait for a connection from netImgui Server (8890 by default)
		// NetImgui Server will try to find running editor client on this port and connect to them
		string kEditorListenPort = "(NetImgui::kDefaultClientPort+1)";
		//---------------------------------------------------------------------

		// Com Port used by Dedicated Server exe to wait for a connection from netImgui Server (8891 by default)
		// NetImgui Server will try to find running dedicaed server client on this port and connect to them
		string kDedicatedServerListenPort = "(NetImgui::kDefaultClientPort+2)";

		//=========================================================================================
		// Plugin setup (no edit should be needed)
		//=========================================================================================

		// Developer modules are automatically loaded only in editor builds but can be stripped out from other builds.
		// Enable runtime loader, if you want this module to be automatically loaded in runtime builds (monolithic).
		bool bEnableRuntimeLoader = true;

		PublicDependencyModuleNames.AddRange( new string[] { "Core", "Projects"} );
		PrivateDependencyModuleNames.AddRange( new string[] { "CoreUObject", "Engine", "Sockets", "ImGui", "NetImguiClient" } );
		PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "../ThirdParty/ImGuiLibrary/Include"),
				Path.Combine(ModuleDirectory, "../ThirdParty/NetImguiClient/Public"),
				Path.Combine(ModuleDirectory, "../ThirdParty/IconFonts/Public"),
			});
		PrivateIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "../ThirdParty/NetImguiClient/Private"),
				Path.Combine(ModuleDirectory, "../ImGui/Private"),
				Path.Combine(ModuleDirectory, "../ThirdParty/IconFonts/Private"),
			});

		//ShadowVariableWarningLevel = WarningLevel.Error;
		PrivateDefinitions.Add("IMGUI_BUNDLE_PYTHON_API");

		//---------------------------------------------------------------------
		// Setup Environment to build with/without netImgui
		//---------------------------------------------------------------------
		PublicDefinitions.Add(string.Format("NETIMGUI_ENABLED={0}", bNetImgui_Enabled ? 1 : 0));
		PublicDefinitions.Add(string.Format("NETIMGUI_FRAMESKIP_ENABLED={0}", bFrameSkip_Enabled ? 1 : 0));
		PublicDefinitions.Add(string.Format("NETIMGUI_WAITCONNECTION_AUTO_ENABLED={0}", bAutoWaitConnection_Enabled ? 1 : 0));
		PublicDefinitions.Add(string.Format("NETIMGUI_FREETYPE_ENABLED={0}", bNetImgui_Enabled && bFreeType_Enabled ? 1 : 0));
		PublicDefinitions.Add(string.Format("NETIMGUI_DEMO_IMGUI_ENABLED={0}", bNetImgui_Enabled && bDemoImgui_Enabled ? 1 : 0));
		PublicDefinitions.Add(string.Format("NETIMGUI_DEMO_ACTOR_ENABLED={0}", bNetImgui_Enabled && bDemoActor_Enabled ? 1 : 0));

		// Network Port configs
		PublicDefinitions.Add("NETIMGUI_CONNECTPORT=" + kRemoteConnectPort);
		PublicDefinitions.Add("NETIMGUI_LISTENPORT_GAME=" + kGameListenPort);
		PublicDefinitions.Add("NETIMGUI_LISTENPORT_EDITOR=" + kEditorListenPort);
		PublicDefinitions.Add("NETIMGUI_LISTENPORT_DEDICATED_SERVER=" + kDedicatedServerListenPort);

		// Misc
		PrivateDefinitions.Add("NETIMGUI_WINSOCKET_ENABLED=0");      // Using Unreal sockets, no need for built-in sockets
		PrivateDefinitions.Add("NETIMGUI_POSIX_SOCKETS_ENABLED=0");  // Using Unreal sockets, no need for built-in sockets

        PrivateDefinitions.Add(string.Format("RUNTIME_LOADER_ENABLED={0}", bEnableRuntimeLoader ? 1 : 0));
	}
}
