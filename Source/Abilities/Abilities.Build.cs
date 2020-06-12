// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

using UnrealBuildTool;

public class Abilities : ModuleRules
{
	public Abilities(ReadOnlyTargetRules Target) : base(Target)
	{
		// Enable support for Include-What-You-Use
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

        PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {});
	}
}
