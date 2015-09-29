#pragma once

#include "TestHeightMapLand.h"
#include "GameFramework/GameMode.h"
#include "TestHeightMapGameMode.generated.h"

/**
 * 
 */
UCLASS(MinimalAPI)
class ATestHeightMapGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:	
	virtual void StartMatch() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "TEST")
	class UTexture2D* HeightMap;

	UPROPERTY(EditDefaultsOnly, Category = "TEST")
	class UMaterial* MeshMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "TEST")
	FVector LandscapeSize;

	UPROPERTY(EditDefaultsOnly, Category = "TEST")
	uint8 PolygonSize;

	UPROPERTY(EditDefaultsOnly, Category = "TEST")
	EQuadOptimizationType OptimizationType;

	UPROPERTY(EditDefaultsOnly, Category = "TEST")
	EDownSamplingType DownSamplingType;
};

