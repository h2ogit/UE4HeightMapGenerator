#include "TestHeightMap.h"
#include "TestHeightMapGameMode.h"
#include "TestHeightMapLand.h"

void ATestHeightMapGameMode::StartMatch()
{
	Super::StartMatch();

	FActorSpawnParameters lSpawnInfo;
	lSpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	lSpawnInfo.bNoFail = true;
	lSpawnInfo.Owner = this;

	FVector lLocation0(0, -600.f, 0.f);
	class ATestHeightMapLand* lLandTest0 = GetWorld()->SpawnActor<ATestHeightMapLand>(ATestHeightMapLand::StaticClass(), lLocation0, FRotator::ZeroRotator, lSpawnInfo);
	if (lLandTest0)
	{
		lLandTest0->GenerateUnoptimizedPattern(HeightMap, MeshMaterial, LandscapeSize, PolygonSize);
	}

	FVector lLandLocation(0.f, 0.f, 0.f);
	class ATestHeightMapLand* lLand = GetWorld()->SpawnActor<ATestHeightMapLand>(ATestHeightMapLand::StaticClass(), lLandLocation, FRotator::ZeroRotator, lSpawnInfo);
	if (lLand)
	{
		lLand->Generate(HeightMap, MeshMaterial, LandscapeSize, PolygonSize, OptimizationType, DownSamplingType);
	}
}


