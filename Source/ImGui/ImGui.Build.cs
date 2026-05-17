// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

public class ImGui : ModuleRules
{
	public ImGui(ReadOnlyTargetRules Target) : base(Target)
	{
		bool bBuildEditor = Target.bBuildEditor;

		// Developer modules are automatically loaded only in editor builds but can be stripped out from other builds.
		// Enable runtime loader, if you want this module to be automatically loaded in runtime builds (monolithic).
		bool bEnableRuntimeLoader = true;
		bool bNetImguiEnabled = true;

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = false;
		CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Error;
		bTreatAsEngineModule = true;

		bUseUnity = false;

		PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "../ThirdParty/ImGuiLibrary/Include"),
				Path.Combine(ModuleDirectory, "../ThirdParty/ImPlotLibrary/Public"),
				Path.Combine(ModuleDirectory, "../ThirdParty/ImGuiFileDialog/Public"),
				Path.Combine(ModuleDirectory, "../ThirdParty/ImTerm/Public"),
				Path.Combine(ModuleDirectory, "../ThirdParty/IconFonts/Public"),
				Path.Combine(ModuleDirectory, "../ThirdParty/ImGuiToggle/Public"),
				Path.Combine(ModuleDirectory, "../ThirdParty/ImGuiColorTextEdit/Public"),
				Path.Combine(ModuleDirectory, "../ThirdParty/ImSpinner/Public"),
				Path.Combine(ModuleDirectory, "../ThirdParty/ImGuiNotify/Public"),
				Path.Combine(ModuleDirectory, "../ThirdParty/NetImguiClient/Public"),
				Path.Combine(ModuleDirectory, "../NetImgui/Public")
				// ... add public include paths required here ...
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"ImGui/Private",
				"ThirdParty/ImGuiLibrary/Private",
				"ThirdParty/ImPlotLibrary/Private",
				"ThirdParty/ImGuiFileDialog/Private",
				"ThirdParty/ImGuiToggle/Private",
				"ThirdParty/ImGuiColorTextEdit/Private",
				"ThirdParty/ImSpinner/Private",
				"ThirdParty/IconFonts/Private",
				// ... add other private include paths required here ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"InputCore",
				"Slate",
				"SlateCore",
				"NetImguiClient"
				// ... add private dependencies that you statically link with here ...
			}
		);


		if (bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"EditorStyle",
					"Settings",
					"UnrealEd",
				}
			);
		}


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				"NetImgui",
				// ... add any modules that your module loads dynamically here ...
			}
		);

		PrivateDefinitions.Add(string.Format("RUNTIME_LOADER_ENABLED={0}", bEnableRuntimeLoader ? 1 : 0));

		// Force ImPlot to export its methods in this module DLL so we can import them in our main project
		PrivateDefinitions.Add("IMPLOT_API=DLLEXPORT");
		PrivateDefinitions.Add("IMPLOT_CUSTOM_NUMERIC_TYPES=(signed char)(unsigned char)(signed short)(unsigned short)(signed int)(unsigned int)(signed long)(unsigned long)(signed long long)(unsigned long long)(float)(double)(long double)");
		PrivateDefinitions.Add("IMGUI_BUNDLE_PYTHON_API");
		PrivateDefinitions.Add("IMGUI_DEFINE_MATH_OPERATORS");
		PublicDefinitions.Add(string.Format("NETIMGUI_ENABLED={0}", bNetImguiEnabled ? 1 : 0));

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDefinitions.Add("IMGUI_DISABLE_FILE_FUNCTIONS"); // Some file functions are not available on Android.
			// Android does not have definitions for the following two macros.
			PrivateDefinitions.Add("TARGET_OS_IPHONE=0");
			PrivateDefinitions.Add("TARGET_OS_OSX=0");
		}

		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Content/..."), StagedFileType.NonUFS);
		RuntimeDependencies.Add("$(EngineDir)/Content/Slate/Fonts/...", StagedFileType.NonUFS);
	}
}
