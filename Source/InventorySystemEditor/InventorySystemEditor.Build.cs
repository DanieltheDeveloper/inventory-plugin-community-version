// © 2024 Daniel Münch. All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class InventorySystemEditor : ModuleRules
{
	public InventorySystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...
				"UMG",
				"UMGEditor",
				"Core",
				"Engine",
				"CoreUObject",
				"InputCore",
				"UnrealEd",
				"Slate",
				"SlateCore",
				"ClassViewer",
				"Projects",
				"InventorySystem",
				"AssetDefinition"
			}
		);
	}
}
