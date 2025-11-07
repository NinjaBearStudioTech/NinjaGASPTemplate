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
			"Chooser",
			"Core", 
			"InputCore", 
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"ModularGameplay",
			"NetCore",
			"PoseSearch",
			"UMG",
			
			// Ninja Bear Studio Modules
			"NinjaCombat",
			"NinjaCombatCore",
			"NinjaCombatInventory",
			"NinjaGAS",
			"NinjaInput",
			"NinjaInventory",
			"NinjaInventoryCore",
			"NinjaInventoryEquipment",
		});
		
		PrivateDependencyModuleNames.AddRange(new []
		{
			"AnimGraphRuntime",
			"AnimationWarpingRuntime",
			"BlendStack",
			"CoreUObject",
			"GameplayCameras",
			"Engine",
			"NinjaInventoryCore",
			"Slate",
			"SlateCore"
		});	
	}
}