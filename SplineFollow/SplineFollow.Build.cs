namespace UnrealBuildTool.Rules
{
	public class SplineFollow : ModuleRules
	{
		public SplineFollow(ReadOnlyTargetRules Target)
			: base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

			PrivateIncludePaths.Add("SplineFollow/Private");

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"GameplayTags"
				}
			);
		}
	}
}