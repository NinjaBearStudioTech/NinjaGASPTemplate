// Ninja Bear Studio Inc., all rights reserved.
using UnrealBuildTool;

public class GASPALS : ModuleRules
{
	public GASPALS(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new [] { "Core", "CoreUObject", "Engine", "InputCore", "UMG" });
		PrivateDependencyModuleNames.AddRange(new [] { "NetCore", "Slate", "SlateCore" });
	}
}
