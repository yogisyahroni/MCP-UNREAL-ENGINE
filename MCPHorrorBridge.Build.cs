using UnrealBuildTool;

public class MCPHorrorBridge : ModuleRules
{
    public MCPHorrorBridge(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[] {
            }
        );
        
        PrivateIncludePaths.AddRange(
            new string[] {
            }
        );
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "HTTP",
                "Json",
                "JsonUtilities",
                "HttpServer",
                "Landscape",
                "PCG",
                "MotionWarping",
                "MetaSoundEngine",
                "AIModule",
                "GameplayTasks",
                "NavigationSystem",
                "LevelSequence",
                "MovieScene",
                "UMG",
                "Slate",
                "SlateCore",
                "Projects"
            }
        );
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "UnrealEd",
                "EditorStyle",
                "PropertyEditor",
                "AssetRegistry",
                "ContentBrowser",
                "BlueprintGraph",
                "KismetCompiler",
                "Kismet",
                "ToolMenus"
            }
        );
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}
