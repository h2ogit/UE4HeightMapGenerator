// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class TestHeightMap : ModuleRules
{
	public TestHeightMap(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(new string[] { 
              "Core"
            , "CoreUObject"
            , "Engine"
            , "InputCore"
            , "RHI"
			, "RenderCore"
            , "ProceduralMeshComponent"
            //, "CustomMeshComponent"
        });
	}
}
