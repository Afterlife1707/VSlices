// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VSlicesTarget : TargetRules
{
	public VSlicesTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("VSlices");
    }
}
