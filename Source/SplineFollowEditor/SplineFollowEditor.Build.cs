namespace UnrealBuildTool.Rules
{
    public class SplineFollowEditor : ModuleRules
    {
        public SplineFollowEditor( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicIncludePathModuleNames.AddRange(
                new string[] 
                {
                    "Engine",
                    "SplineFollow",
                }
            );

            PrivateIncludePaths.AddRange(
                new string[]
                { 
                    "SplineFollowEditor/Private",
                }
            );

            PublicIncludePaths.AddRange(
                new string[]
                {
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Engine",
                    "Core",
                    "CoreUObject",
                    "UnrealEd",
                    "ComponentVisualizers",
                    "SplineFollow"
                }
            );
        }
    }
}