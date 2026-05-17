// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include <Modules/ModuleManager.h>

#ifndef NETIMGUI_ENABLED
	#define NETIMGUI_ENABLED 0

#elif NETIMGUI_ENABLED

#include "NetImgui_Api.h"

//=================================================================================================
// Additional Header includes
// Note: The following 'Defines' are optional features enabled by user in NetImgui.Build.cs
//=================================================================================================
#include "imgui.h"

#endif //NETIMGUI_ENABLED

class FImGuiContextProxy;

//=================================================================================================
// NETIMGUI Module
//=================================================================================================
class FNetImguiModule : public IModuleInterface
{
public:
	/**
	 * Singleton-like access to this module's interface. This is just for convenience!
	 * Beware of calling this during the shutdown phase, though. Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FNetImguiModule&	Get() {
		// Avoid lookup by finding the element once per 'Frame/Dll' and storing the pointer
		static FNetImguiModule* LoadedModulePtr = nullptr;
		static uint64 sLastFrame = 0;
		if( !LoadedModulePtr || sLastFrame != GFrameCounter ){
			LoadedModulePtr = static_cast<FNetImguiModule*>(&FModuleManager::LoadModuleChecked<FNetImguiModule>("NetImgui"));
		}
		return *LoadedModulePtr;
	}

	/**
	 * Checks to see if this module is loaded and ready. It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static bool	IsAvailable() { return FModuleManager::Get().IsModuleLoaded("NetImgui"); }

	//=================================================================================================
	// Pick the most appropriate port to wait for connection
	//=================================================================================================
	static uint32 GetListeningPort()
	{
		if (IsRunningDedicatedServer())
		{
			return NETIMGUI_LISTENPORT_DEDICATED_SERVER;
		}
		else if (FApp::IsGame())
		{
			return NETIMGUI_LISTENPORT_GAME;
		}
		else
		{
			return NETIMGUI_LISTENPORT_EDITOR;
		}
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#if NETIMGUI_ENABLED

	virtual void TryConnectingToServer(const FString& HostnameAndPort);
	virtual void TryListeningForServer(const FString& ListeningPort);

	virtual void BuildFont(float InFontDPIScale);

	enum class EDualUI : uint8
	{
		DualDisplay_Off,	// Only draw to the main display
		DualDisplay_On,		// Draw to both main and remote display
	};

	/**
	* Tell us if the plugin has a working connection established with NetImgui remote server
	*
	* @return True if the module is connected to a NetImgui remote server
	*/
	virtual bool IsConnected() const;

public:
	/**
	* Use this method when drawing Dear ImGui content on the GameThread.
	* It is not required when drawing is happening inside a 'OnDrawImgui' callback.
	*
	* With 'FrameSkip' enabled, there are frames where we are not waiting on new
	* Dear Imgui drawing, and attempting to do so will result in an error.
	*
	* @return True if the module is expecting some Dear ImGui draws this frame
	*/
	bool IsDrawing() const
	{
		checkSlow(IsInGameThread());
		if (NetImgui::IsDrawing() && bIsDrawing)
		{
			// HotReload note: When a dll is reloaded, original dll is still loaded and all 'Dear ImGui'
			// functions are still pointing to it when called from outside this dll. Only this module object
			// is recreated. This means that the game code will call original dll but this module object
			// will use reloaded dll ImGui functions. To prevent issue with destroyed context, we are
			// making sure that the original dll knows about this module's newly created context here.
			ImGui::SetCurrentContext(NetImguiContextPtr);
			return true;
		}
		return false;
	}

	virtual void NewFrame();

	virtual void EndFrame(FImGuiContextProxy*);

	//=================================================================================================
	// (Public) Set the remote ImGui context as current if provided Proxy context
	//			is the one we are assigned to override
	//=================================================================================================
	virtual bool SetupDrawAtRemote();

	virtual void SendDataTexture(ImTextureID textureId, void* pData, uint16_t width, uint16_t height, NetImgui::eTexFormat format);

	/**
	* Add your Dear ImGui drawing callbacks to this emitter
	** Note: If NetImgui module is reloaded, you will lose your callbacks
	*/
	FSimpleMulticastDelegate		OnDrawImgui;

protected:
	inline static EDualUI			DualUIType = EDualUI::DualDisplay_Off;		// Are we drawing to both main and remote display?
	ImGuiContext*					NetImguiContextPtr = nullptr;
	FDelegateHandle					OnCheckCallback;
	bool							bIsDrawing = false;
	float							FontDPIScale = 0.f;

#endif //NETIMGUI_ENABLED
};
