// Copyright (c) 2023 Bibby. All Rights Reserved.

using UnrealBuildTool;

public class NetImguiClient : ModuleRules
{
	public NetImguiClient(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		//---------------------------------------------------------------------
		// Settings configuration
		//---------------------------------------------------------------------
		bool bUseNetImgui			= true;									// Toggle netImgui enabled here
		string kGameListenPort		= "(NetImgui::kDefaultClientPort)";		// Com Port used by Game exe to wait for a connection from netImgui Server (8889 by default)
		string kEditorListenPort	= "(NetImgui::kDefaultClientPort+1)";   // Com Port used by Editor exe to wait for a connection from netImgui Server (8890 by default)
		string kServerListenPort	= "(NetImgui::kDefaultClientPort+2)";   // Com Port used by Server exe to wait for a connection from netImgui Server (8891 by default)

		if (bUseNetImgui)
		{
			PublicDependencyModuleNames.Add("Sockets");

			PublicDefinitions.Add("NETIMGUI_ENABLED=1");
			PublicDefinitions.Add("NETIMGUI_LISTENPORT_GAME=" + kGameListenPort);
			PublicDefinitions.Add("NETIMGUI_LISTENPORT_EDITOR=" + kEditorListenPort);
			PublicDefinitions.Add("NETIMGUI_LISTENPORT_DEDICATED_SERVER=" + kServerListenPort);
			PublicDefinitions.Add("NETIMGUI_IMGUI_CALLBACK_ENABLED=0");	// Disabled NetImgui intercepting the NewFrame/Render of Imgui Contexts
			PublicDefinitions.Add("NETIMGUI_WINSOCKET_ENABLED=0");      // Using Unreal sockets, no need for built-in sockets
			PublicDefinitions.Add("NETIMGUI_POSIX_SOCKETS_ENABLED=0");  // Using Unreal sockets, no need for built-in sockets
		}
		else
		{
			PublicDefinitions.Add("NETIMGUI_ENABLED=0");
		}
	}
}
