// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class AbilitiesTest : ModuleRules
	{
		public AbilitiesTest(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			bEnforceIWYU = true;
            bLegacyPublicIncludePaths = false;

            PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"Engine",
				"CoreUObject",
                "Automatron",
                "Abilities"
			});

			PrivateDependencyModuleNames.AddRange(new string[]{});
		}
	}
}