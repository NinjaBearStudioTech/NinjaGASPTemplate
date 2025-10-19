// Ninja Bear Studio Inc., all rights reserved.
using UnrealBuildTool;

public class NinjaGASP : ModuleRules
{
	public NinjaGASP(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new []
		{
			// Unreal Engine Modules
			"AIModule",
			"Core", 
			"InputCore", 
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"ModularGameplay",
			"NetCore",
			"UMG",
			
			// Ninja Bear Studio Modules
			"NinjaCombat",
			"NinjaGAS",
			"NinjaInput"
		});
		
		PrivateDependencyModuleNames.AddRange(new []
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore"
		});	
	}
}