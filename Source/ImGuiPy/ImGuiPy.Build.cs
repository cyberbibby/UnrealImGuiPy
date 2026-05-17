// Copyright (c) 2024 Bibby. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ImGuiPy : ModuleRules
{
    public ImGuiPy(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bUseUnity = false;
        bUseRTTI = true;
        bEnableExceptions = true;
        bIgnoreUnresolvedSymbols = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "ImGui",
                "Python3",
            }
        );

        PublicIncludePaths.AddRange(
	        new string[]
	        {
		        Path.Combine(ModuleDirectory, "../ThirdParty/ImGuiLibrary/Include"),
		        Path.Combine(ModuleDirectory, "../ThirdParty/ImPlotLibrary/Public"),
		        Path.Combine(ModuleDirectory, "../ThirdParty/ImGuiColorTextEdit/Public"),
		        Path.Combine(ModuleDirectory, "../ThirdParty/ImGuiToggle/Public"),
		        Path.Combine(ModuleDirectory, "../ThirdParty/ImSpinner/Public"),
		        Path.Combine(ModuleDirectory, "../ThirdParty/PyBind11/Include"),
		        // ... add public include paths required here ...
	        }
        );

        PrivateIncludePaths.AddRange(
	        new string[]
	        {
		        "ThirdParty/ImGuiLibrary/Private",
		        "ThirdParty/ImPlotLibrary/Private",
		        "ThirdParty/ImGuiToggle/Private",
		        "ThirdParty/ImGuiColorTextEdit/Private",
		        "ThirdParty/ImSpinner/Private",
		        // ... add other private include paths required here ...
	        }
        );

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
	        PrivateDefinitions.Add("IMGUI_DISABLE_FILE_FUNCTIONS"); // Some file functions are not available on Android.
        }

        PrivateDefinitions.Add("IMGUI_BUNDLE_WITH_IMPLOT");
        PrivateDefinitions.Add("IMGUI_BUNDLE_PYTHON_API");
        PrivateDefinitions.Add("PYBIND11_DETAILED_ERROR_MESSAGES");
    }
}
