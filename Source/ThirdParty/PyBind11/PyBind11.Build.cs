// Copyright (c) 2023 Bibby. All Rights Reserved.

using UnrealBuildTool;

public class PyBind11 : ModuleRules
{
	public PyBind11(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		CppStandard = CppStandardVersion.Cpp17;

		bEnableExceptions = true;
		bUseRTTI = true;
	}
}
