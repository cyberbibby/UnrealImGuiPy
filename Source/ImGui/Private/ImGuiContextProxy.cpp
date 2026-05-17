// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiContextProxy.h"

#include "ImGuiDelegatesContainer.h"
#include "ImGuiImplementation.h"
#include "ImGuiInteroperability.h"
#include "Utilities/Arrays.h"
#include "VersionCompatibility.h"

// Include ImPlot here so we can call `ImPlot::CreateContext`
#include <implot.h>

// Include Icons
#include <IconsFontAwesome6.h>
#include <IconsMaterialDesign.h>
#include <IconsKenney.h>
#include "FontKenney/KenneyIcon.cpp"

#include <GenericPlatform/GenericPlatformFile.h>
#include <HAL/FileManager.h>
#include <Misc/Paths.h>
#include <Interfaces/IPluginManager.h>

#include <NetImguiModule.h>

#include "imgui_internal.h"


static constexpr float DEFAULT_CANVAS_WIDTH = 3840.f;
static constexpr float DEFAULT_CANVAS_HEIGHT = 2160.f;


namespace
{
	FString GetSaveDirectory()
	{
#if ENGINE_COMPATIBILITY_LEGACY_SAVED_DIR
		const FString SavedDir = FPaths::GameSavedDir();
#else
		const FString SavedDir = FPaths::ProjectSavedDir();
#endif

		FString Directory = FPaths::Combine(*SavedDir, TEXT("ImGui"));

		// Make sure that directory is created.
		IPlatformFile::GetPlatformPhysical().CreateDirectory(*Directory);

		return Directory;
	}

	FString GetIniFile(const FString& Name)
	{
		static FString SaveDirectory = GetSaveDirectory();
		return FPaths::Combine(SaveDirectory, Name + TEXT(".ini"));
	}

	struct FGuardCurrentContext
	{
		FGuardCurrentContext()
			: OldContext(ImGui::GetCurrentContext())
		{
		}

		~FGuardCurrentContext()
		{
			if (bRestore)
			{
				ImGui::SetCurrentContext(OldContext);
			}
		}

		FGuardCurrentContext(FGuardCurrentContext&& Other)
			: OldContext(MoveTemp(Other.OldContext))
		{
			Other.bRestore = false;
		}

		FGuardCurrentContext& operator=(FGuardCurrentContext&&) = delete;

		FGuardCurrentContext(const FGuardCurrentContext&) = delete;
		FGuardCurrentContext& operator=(const FGuardCurrentContext&) = delete;

	private:

		ImGuiContext* OldContext = nullptr;
		bool bRestore = true;
	};
}

FImGuiContextProxy::FImGuiContextProxy(const FString& InName, int32 InContextIndex, ImFontAtlas* InFontAtlas, float InDPIScale)
	: Name(InName)
	, ContextIndex(InContextIndex)
	, IniFilename(TCHAR_TO_ANSI(*GetIniFile(InName)))
{
	// Create context.
	Context = ImGui::CreateContext(InFontAtlas);

	// Create ImPlot context
	ImPlot::CreateContext();

	// Set this context in ImGui for initialization (any allocations will be tracked in this context).
	SetAsCurrent();

	// Start initialization.
	ImGuiIO& IO = ImGui::GetIO();

	// Set session data storage.
	IO.IniFilename = IniFilename.c_str();

	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Initialize UE's default font that support Chinese as ImGui default font.
	InitCustomFonts(IO, InDPIScale);

	// Start with the default canvas size.
	ResetDisplaySize();
	IO.DisplaySize = {(float)DisplaySize.X, (float)DisplaySize.Y};

	// Set the initial DPI scale.
	SetDPIScale(InDPIScale);

	// Initialize key mapping, so context can correctly interpret input state.
	ImGuiInterops::SetUnrealKeyMap();

	// Begin frame to complete context initialization (this is to avoid problems with other systems calling to ImGui
	// during startup).
	BeginFrame();
}

FImGuiContextProxy::~FImGuiContextProxy()
{
	if (Context)
	{
		// It seems that to properly shutdown context we need to set it as the current one (at least in this framework
		// version), even though we can pass it to the destroy function.
		SetAsCurrent();

		// Ensure frame has ended
		EndFrame();

		// Save context data and destroy.
		ImGui::DestroyContext(Context);

		// Destroy ImPlot context
		ImPlot::DestroyContext();
	}
}

void FImGuiContextProxy::InitCustomFonts(ImGuiIO& IO, const float InDPIScale) const
{
	ImFontConfig FontConfig;
	FontConfig.FontDataOwnedByAtlas = false;

	ImFont* DefaultFont = IO.Fonts->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
		TCHAR_TO_ANSI(*FPaths::Combine(FPaths::EngineContentDir(), TEXT("Slate"), TEXT("Fonts"), TEXT("Roboto-Regular.ttf"))),
#else
		TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
			*FPaths::Combine(FPaths::EngineContentDir(), TEXT("Slate"), TEXT("Fonts"), TEXT("Roboto-Regular.ttf")))),
#endif
		FMath::RoundToFloat(16.f * InDPIScale), &FontConfig, IO.Fonts->GetGlyphRangesDefault());

	FontConfig.MergeMode = true;
	FontConfig.PixelSnapH = true;

	IO.Fonts->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
		TCHAR_TO_ANSI(*FPaths::Combine(FPaths::EngineContentDir(), TEXT("Slate"), TEXT("Fonts"), TEXT("DroidSansFallback.ttf"))),
#else
		TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
			*FPaths::Combine(FPaths::EngineContentDir(), TEXT("Slate"), TEXT("Fonts"), TEXT("DroidSansFallback.ttf")))),
#endif
		FMath::RoundToFloat(16.f * InDPIScale), &FontConfig, IO.Fonts->GetGlyphRangesChineseFull());

	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ImGui"));
	constexpr ImWchar IconRanges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
	FontConfig.GlyphOffset.y = 3.f;
	FontConfig.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
	IO.Fonts->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
		TCHAR_TO_ANSI(*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT(FONT_ICON_FILE_NAME_FAS))),
#else
		TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
			*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT(FONT_ICON_FILE_NAME_FAS)))),
#endif
		FMath::RoundToFloat(16.f * InDPIScale), &FontConfig, IconRanges); // Font Awesome 6 Icons

	constexpr ImWchar MatIconRanges[] = {ICON_MIN_MD, ICON_MAX_16_MD, 0};
	FontConfig.GlyphOffset.y = 5.f;
	IO.Fonts->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
		TCHAR_TO_ANSI(*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT(FONT_ICON_FILE_NAME_MD))),
#else
		TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
			*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT(FONT_ICON_FILE_NAME_MD)))),
#endif
		FMath::RoundToFloat(16.f * InDPIScale), &FontConfig, MatIconRanges); // Google's Material Design icons

	constexpr ImWchar KenneyIconRanges[] = {ICON_MIN_KI, ICON_MAX_KI, 0};
	FontConfig.GlyphOffset.y = 3.f;
	IO.Fonts->AddFontFromMemoryCompressedTTF(KenneyIcon_compressed_data, KenneyIcon_compressed_size,
		FMath::RoundToFloat(16.f * InDPIScale), &FontConfig, KenneyIconRanges);

	IO.Fonts->AddFontFromFileTTF(
#if PLATFORM_WINDOWS
		TCHAR_TO_ANSI(*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT("JetBrainsMonoNL-Medium.ttf"))),
#else
		TCHAR_TO_ANSI(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(
			*FPaths::Combine(Plugin->GetContentDir(), TEXT("Fonts"), TEXT("JetBrainsMonoNL-Medium.ttf")))),
#endif
		18.0f);

	IO.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
	IO.Fonts->Build();
	IO.FontDefault = DefaultFont;
}

void FImGuiContextProxy::ResetDisplaySize()
{
	DisplaySize = { DEFAULT_CANVAS_WIDTH, DEFAULT_CANVAS_HEIGHT };
}

void FImGuiContextProxy::SetDPIScale(float Scale)
{
	if (DPIScale != Scale)
	{
		DPIScale = Scale;

		ImGuiStyle NewStyle = ImGuiStyle();
		NewStyle.ScaleAllSizes(Scale);

		FGuardCurrentContext GuardContext;
		SetAsCurrent();
		ImGui::GetStyle() = MoveTemp(NewStyle);
	}
}

void FImGuiContextProxy::DrawEarlyDebug()
{
	if (bIsFrameStarted && !bIsDrawEarlyDebugCalled)
	{
		bIsDrawEarlyDebugCalled = true;
		if (FNetImguiModule::Get().IsConnected())
		{
			if (FNetImguiModule::Get().SetupDrawAtRemote())
			{
				BroadcastMultiContextEarlyDebug();
				BroadcastWorldEarlyDebug();
			}
		}
		else
		{
			SetAsCurrent();

			// Delegates called in order specified in FImGuiDelegates.
			BroadcastMultiContextEarlyDebug();
			BroadcastWorldEarlyDebug();
		}
	}
}

void FImGuiContextProxy::DrawDebug()
{
	if (bIsFrameStarted && !bIsDrawDebugCalled)
	{
		bIsDrawDebugCalled = true;

		// Make sure that early debug is always called first to guarantee order specified in FImGuiDelegates.
		DrawEarlyDebug();

		if (FNetImguiModule::Get().IsConnected())
		{
			if (FNetImguiModule::Get().SetupDrawAtRemote())
			{
				BroadcastWorldDebug();
				BroadcastMultiContextDebug();
			}
		}
		else
		{
			SetAsCurrent();

			// Delegates called in order specified in FImGuiDelegates.
			BroadcastWorldDebug();
			BroadcastMultiContextDebug();
		}
	}
}

void FImGuiContextProxy::Tick(float DeltaSeconds)
{
	// Making sure that we tick only once per frame.
	if (LastFrameNumber < GFrameNumber)
	{
		LastFrameNumber = GFrameNumber;
#if !NETIMGUI_ENABLED
		SetAsCurrent();
#endif
		if (bIsFrameStarted)
		{
			// Make sure that draw events are called before the end of the frame.
			DrawDebug();

			// Ending frame will produce render output that we capture and store for later use. This also puts context to
			// state in which it does not allow to draw controls, so we want to immediately start a new frame.
			EndFrame();
		}

		ImGuiIO& IO = ImGui::GetIO();

		// Update context information (some data need to be collected before starting a new frame while some other data
		// may need to be collected after).
		bHasActiveItem = ImGui::IsAnyItemActive();
		MouseCursor = ImGuiInterops::ToSlateMouseCursor(ImGui::GetMouseCursor());

		// Update remaining context information.
		bWantsMouseCapture = IO.WantCaptureMouse;

		// Set Config and Backend flags in this IO context, as InputState IO is only for function use
		ImGuiInterops::SetFlag(IO.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard, InputState.IsKeyboardNavigationEnabled());
		ImGuiInterops::SetFlag(IO.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad, InputState.IsGamepadNavigationEnabled());
		ImGuiInterops::SetFlag(IO.BackendFlags, ImGuiBackendFlags_HasGamepad, InputState.HasGamepad());

		// Begin a new frame and set the context back to a state in which it allows to draw controls.
		BeginFrame(DeltaSeconds);
	}
}

void FImGuiContextProxy::BeginFrame(float DeltaTime)
{
	if (!bIsFrameStarted)
	{
		if (FNetImguiModule::Get().IsConnected())
		{
			FNetImguiModule::Get().NewFrame();
		}
		else
		{
			SetAsCurrent();

			ImGuiIO& IO = ImGui::GetIO();
			IO.DeltaTime = DeltaTime;
			IO.DisplaySize = ImVec2(DisplaySize.X, DisplaySize.Y);

			InputState.ClearUpdateState();

			ImGui::NewFrame();
		}

		bIsFrameStarted = true;
		bIsDrawEarlyDebugCalled = false;
		bIsDrawDebugCalled = false;
	}
}

void FImGuiContextProxy::EndFrame()
{
	if (bIsFrameStarted)
	{
		if (FNetImguiModule::Get().IsConnected())
		{
			FNetImguiModule::Get().EndFrame(this);
		}
		else
		{
			SetAsCurrent();

			if (ImGui::GetCurrentContext()->WithinFrameScope)
			{
				// Prepare draw data (after this call we cannot draw to this context until we start a new frame).
				ImGui::Render();

				// Update our draw data, so we can use them later during Slate rendering while ImGui is in the middle of the
				// next frame.
				UpdateDrawData(ImGui::GetDrawData());
			}
		}

		bIsFrameStarted = false;
	}
}

void FImGuiContextProxy::UpdateDrawData(ImDrawData* DrawData)
{
	if (DrawData && DrawData->CmdListsCount > 0)
	{
#if ENGINE_COMPATIBILITY_LEGACY_CONTAINER_SHRINKING
		DrawLists.SetNum(DrawData->CmdListsCount, false);
#else
		DrawLists.SetNum(DrawData->CmdListsCount, EAllowShrinking::No);
#endif // ENGINE_COMPATIBILITY_LEGACY_CONTAINER_SHRINKING

		for (int Index = 0; Index < DrawData->CmdListsCount; Index++)
		{
			DrawLists[Index].TransferDrawData(*DrawData->CmdLists[Index]);
		}
	}
	else
	{
		// If we are not rendering then this might be a good moment to empty the array.
		DrawLists.Empty();
	}
}

void FImGuiContextProxy::BroadcastWorldEarlyDebug()
{
	if (ContextIndex != Utilities::INVALID_CONTEXT_INDEX)
	{
		FSimpleMulticastDelegate& WorldEarlyDebugEvent = FImGuiDelegatesContainer::Get().OnWorldEarlyDebug(ContextIndex);
		if (WorldEarlyDebugEvent.IsBound())
		{
			WorldEarlyDebugEvent.Broadcast();
		}
	}
}

void FImGuiContextProxy::BroadcastMultiContextEarlyDebug()
{
	FSimpleMulticastDelegate& MultiContextEarlyDebugEvent = FImGuiDelegatesContainer::Get().OnMultiContextEarlyDebug();
	if (MultiContextEarlyDebugEvent.IsBound())
	{
		MultiContextEarlyDebugEvent.Broadcast();
	}
}

void FImGuiContextProxy::BroadcastWorldDebug()
{
	if (DrawEvent.IsBound())
	{
		DrawEvent.Broadcast();
	}

	if (ContextIndex != Utilities::INVALID_CONTEXT_INDEX)
	{
		FSimpleMulticastDelegate& WorldDebugEvent = FImGuiDelegatesContainer::Get().OnWorldDebug(ContextIndex);
		if (WorldDebugEvent.IsBound())
		{
			WorldDebugEvent.Broadcast();
		}
	}
}

void FImGuiContextProxy::BroadcastMultiContextDebug()
{
	FSimpleMulticastDelegate& MultiContextDebugEvent = FImGuiDelegatesContainer::Get().OnMultiContextDebug();
	if (MultiContextDebugEvent.IsBound())
	{
		MultiContextDebugEvent.Broadcast();
	}
}
