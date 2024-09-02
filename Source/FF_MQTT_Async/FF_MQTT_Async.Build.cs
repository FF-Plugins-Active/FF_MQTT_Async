// Some copyright should be here...

using System;
using System.IO;
using UnrealBuildTool;

public class FF_MQTT_Async : ModuleRules
{
	public FF_MQTT_Async(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        bEnableUndefinedIdentifierWarnings = false;
        bEnableExceptions = true;

        if (UnrealTargetPlatform.Win64 == Target.Platform)
        {
            bUseRTTI = true;

			// C Library
            PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "paho_c", "Win64", "include"));

            // Asynchronous with SSL
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty", "paho_c", "Win64", "lib", "paho-mqtt3as-static.lib"));
        }

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "OpenSSL",
                "FF_MBedTLS",
            }
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Json",
                "JsonUtilities",
                "JsonBlueprintUtilities",
            }
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			}
			);
	}
}
