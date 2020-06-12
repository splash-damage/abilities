// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class AbilitiesEditor : ModuleRules
	{
		public AbilitiesEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			bEnforceIWYU = true;
            bLegacyPublicIncludePaths = false;

            PublicDependencyModuleNames.AddRange( new string[]
			{
				"Core",
				"Engine",
				"CoreUObject",
				"Abilities"
			});

			PrivateDependencyModuleNames.AddRange( new string[]
			{
				"AssetTools",
				"Projects",
				"UnrealEd",
				"SlateCore",
				"Slate",
				"EditorStyle",
			});
		}
	}
}