namespace UnrealBuildTool.Rules
{
	public class SplineFollow : ModuleRules
	{
		public SplineFollow(ReadOnlyTargetRules Target)
			: base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"GameplayTags",
					"GameplayAbilities",
					"GameplayTasks"
				}
			);
		}
	}
}