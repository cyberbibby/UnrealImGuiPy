// Copyright (c) 2024 Bibby. All Rights Reserved.

using UnrealBuildTool;

public class NetImguiEditorToolbar : ModuleRules
{
    public NetImguiEditorToolbar(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
                "ToolMenus",
                "Projects",
            }
        );
    }
}
