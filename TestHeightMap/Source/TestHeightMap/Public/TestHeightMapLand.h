#pragma once
#include "ProceduralMeshComponent.h"
#include "TestHeightMapLand.generated.h"

UENUM(NotBlueprintType, NotBlueprintable)
enum class EQuadOptimizationType : uint8
{
	None, // without optimization
	AxesAlternately, // 2 pass: first Y then X
	FrontAndBack, // 1 pass: front on Y and back on X
	Square, // 1 pass: square
	Rectangle, // 1 pass: rectangle
};

UENUM(NotBlueprintType, NotBlueprintable)
enum class EDownSamplingType : uint8
{
	None = 1, // Same as original
	TwoTimes = 2, // 2 times lower
	FourTimes = 4, // 4 times lower
};

USTRUCT(NotBlueprintType, NotBlueprintable)
struct FVertice
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	FVector Loc;

	UPROPERTY()
	FColor Color;
	
	FVertice()
	{
		Loc = FVector::ZeroVector;
		Color = FColor::White;
	}

	bool operator==(const FVertice& OtherVertice) const
	{
		if ((Loc == OtherVertice.Loc) && (Color == OtherVertice.Color))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

USTRUCT(NotBlueprintType, NotBlueprintable)
struct FQuad
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	int32 V1;

	UPROPERTY()
	int32 V2;

	UPROPERTY()
	int32 V3;

	UPROPERTY()
	int32 V4;

	UPROPERTY()
	bool bUsed;

	FQuad()
	{
		V1 = -1;
		V2 = -1;
		V3 = -1;
		V4 = -1;
		bUsed = false;
	}

	float GetQuadSize(TArray<FVertice>& Vertices) const
	{
		return (Vertices[V1].Loc + Vertices[V2].Loc + Vertices[V3].Loc + Vertices[V4].Loc).Size();
	}

	bool HasSameSide(const FQuad& OtherQuad) const
	{
		if ((OtherQuad.V1 == V4) && (OtherQuad.V2 == V3))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool HasSameHeight(const FQuad& OtherQuad, TArray<FVertice>& Vertices) const
	{
		if ((Vertices[OtherQuad.V1].Loc.Z == Vertices[V1].Loc.Z) && (Vertices[OtherQuad.V2].Loc.Z == Vertices[V2].Loc.Z) && (Vertices[OtherQuad.V3].Loc.Z == Vertices[V3].Loc.Z) && (Vertices[OtherQuad.V4].Loc.Z == Vertices[V4].Loc.Z))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

/**
 * 
 */
UCLASS()
class ATestHeightMapLand : public AActor
{
	GENERATED_BODY()
	
public:
	ATestHeightMapLand(const FObjectInitializer& ObjectInitializer);

	void GenerateUnoptimizedPattern(class UTexture2D* aHeightMap, class UMaterial* aMat, FVector aSize, int32 PolygonSize);

	void Generate(class UTexture2D* aHeightMap, class UMaterial* aMat, FVector aSize, int32 PolygonSize, EQuadOptimizationType OptimizationType, EDownSamplingType DownSamplingType);

protected:
	UPROPERTY(Transient)
	class UProceduralMeshComponent* Mesh;

private:
	void GenerateMesh(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents, TArray<FColor>& VertexColors, class UTexture2D* aHeightMap, int32 PolygonSize, EQuadOptimizationType OptimizationType, EDownSamplingType DownSamplingType);

	void OptimizeMeshQuads_AxesAlternately(TArray<FQuad>& NormalQuads, TArray<FVertice>& NormalVertices, FIntPoint& TextureSize, TArray<FQuad>& OptimizedQuads);
	void OptimizeMeshQuads_Square(TArray<FQuad>& NormalQuads, TArray<FVertice>& NormalVertices, FIntPoint& TextureSize, TArray<FQuad>& OptimizedQuads);
	void OptimizeMeshQuads_Rectangle(TArray<FQuad>& NormalQuads, TArray<FVertice>& NormalVertices, FIntPoint& TextureSize, TArray<FQuad>& OptimizedQuads);
	void OptimizeMeshQuads_FrontAndBack(TArray<FQuad>& NormalQuads, TArray<FVertice>& NormalVertices, FIntPoint& TextureSize, TArray<FQuad>& OptimizedQuads);
};
