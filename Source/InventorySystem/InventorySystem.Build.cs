// © 2024 Daniel Münch. All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class InventorySystem : ModuleRules
{
	public InventorySystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add other public dependencies that you statically link with here ...
				"UMG",
				"Core",
				"Engine",
				"CoreUObject",
				"InputCore",
				"Slate",
				"SlateCore",
				"Projects"
            }
		);
	}
}
