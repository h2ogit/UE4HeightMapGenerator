#include "TestHeightMap.h"
#include "TestHeightMapLand.h"
#include "ProceduralMeshComponent.h"

ATestHeightMapLand::ATestHeightMapLand(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Mesh = ObjectInitializer.CreateDefaultSubobject<UProceduralMeshComponent>(this, TEXT("Mesh0"));
	if (Mesh)
	{
		Mesh->bReceivesDecals = false;
		Mesh->CastShadow = true;
		Mesh->SetCollisionObjectType(ECC_WorldDynamic);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->UpdateCollisionProfile();
		RootComponent = Mesh;
	}

	this->SetActorEnableCollision(true);
}

void ATestHeightMapLand::GenerateUnoptimizedPattern(class UTexture2D* aHeightMap, class UMaterial* aMat, FVector aSize, int32 PolygonSize)
{
	Mesh->ClearAllMeshSections();

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;

	FIntPoint lTextureSize = FIntPoint(aHeightMap->GetSizeX(), aHeightMap->GetSizeY());

	Vertices.Reset();
	VertexColors.Reset();

	FTexture2DMipMap* MyMipMap = &aHeightMap->PlatformData->Mips[0];
	FByteBulkData* RawImageData = &MyMipMap->BulkData;
	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

	for (int32 x = 0; x < lTextureSize.X; x++)
	{
		for (int32 y = 0; y < lTextureSize.Y; y++)
		{
			FColor lPixelColor = FormatedImageData[y * lTextureSize.X + x];

			float lHeight0 = lPixelColor.R;

			FVector lVertice = FVector(x * PolygonSize, y * PolygonSize, lHeight0);

			Vertices.Add(lVertice);
			VertexColors.Add(lPixelColor);
		}
	}

	RawImageData->Unlock();

	Triangles.Reset();

	int32 lVerticesQuantity = Vertices.Num();

	Normals.Reset();
	Normals.AddUninitialized(lVerticesQuantity);

	Tangents.Reset();
	Tangents.AddUninitialized(lVerticesQuantity);

	UVs.Reset();
	UVs.AddUninitialized(lVerticesQuantity);

	const FVector2D UVTextureScale = FVector2D(1.f / float(lTextureSize.X), 1.f / float(lTextureSize.Y));

	for (int32 i = 0; i < lVerticesQuantity - lTextureSize.Y - 1; i++)
	{
		int32 CurrentY = i % lTextureSize.Y;
		if (CurrentY == lTextureSize.Y - 1)
		{
			continue;
		}

		int VertIndex1 = i;
		int VertIndex2 = VertIndex1 + 1;
		int VertIndex3 = i + lTextureSize.Y + 1;
		int VertIndex4 = i + lTextureSize.Y;

		Triangles.Add(VertIndex1);
		Triangles.Add(VertIndex2);
		Triangles.Add(VertIndex3);

		Triangles.Add(VertIndex1);
		Triangles.Add(VertIndex3);
		Triangles.Add(VertIndex4);

		UVs[VertIndex1] = FVector2D(UVTextureScale.X * Vertices[VertIndex1].X / PolygonSize, UVTextureScale.Y * Vertices[VertIndex1].Y / PolygonSize);
		UVs[VertIndex2] = FVector2D(UVTextureScale.X * Vertices[VertIndex2].X / PolygonSize, UVTextureScale.Y * Vertices[VertIndex2].Y / PolygonSize);
		UVs[VertIndex3] = FVector2D(UVTextureScale.X * Vertices[VertIndex3].X / PolygonSize, UVTextureScale.Y * Vertices[VertIndex3].Y / PolygonSize);
		UVs[VertIndex4] = FVector2D(UVTextureScale.X * Vertices[VertIndex4].X / PolygonSize, UVTextureScale.Y * Vertices[VertIndex4].Y / PolygonSize);

		Normals[VertIndex1] = FVector::CrossProduct(Vertices[VertIndex1] - Vertices[VertIndex4], Vertices[VertIndex1] - Vertices[VertIndex2]).GetSafeNormal();
		Normals[VertIndex2] = FVector::CrossProduct(Vertices[VertIndex2] - Vertices[VertIndex1], Vertices[VertIndex2] - Vertices[VertIndex3]).GetSafeNormal();
		Normals[VertIndex3] = FVector::CrossProduct(Vertices[VertIndex3] - Vertices[VertIndex2], Vertices[VertIndex3] - Vertices[VertIndex4]).GetSafeNormal();
		Normals[VertIndex4] = FVector::CrossProduct(Vertices[VertIndex4] - Vertices[VertIndex3], Vertices[VertIndex4] - Vertices[VertIndex1]).GetSafeNormal();

		Tangents[VertIndex1] = FProcMeshTangent((Vertices[VertIndex1] - Vertices[VertIndex2]).GetSafeNormal(), true);
		Tangents[VertIndex2] = FProcMeshTangent((Vertices[VertIndex2] - Vertices[VertIndex3]).GetSafeNormal(), true);
		Tangents[VertIndex3] = FProcMeshTangent((Vertices[VertIndex3] - Vertices[VertIndex4]).GetSafeNormal(), true);
		Tangents[VertIndex4] = FProcMeshTangent((Vertices[VertIndex4] - Vertices[VertIndex1]).GetSafeNormal(), true);
	}

	Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);
	Mesh->SetMaterial(0, aMat);

	FVector lMeshSize = Mesh->Bounds.GetBox().GetSize();
	FVector lNewScale = FVector((aSize.X / lMeshSize.X), (aSize.Y / lMeshSize.Y), (aSize.Z / lMeshSize.Z));
	Mesh->SetWorldScale3D(lNewScale);

	Mesh->ShouldGenerateAutoLOD();

	UE_LOG(LogClass, Log, TEXT("Generated pattern: Verticles = %i | Triangles = %i | UVs = %i | Normals = %i | Tangents = %i"), Vertices.Num(), Triangles.Num(), UVs.Num(), Normals.Num(), Tangents.Num());
}

void ATestHeightMapLand::Generate(class UTexture2D* aHeightMap, class UMaterial* aMat, FVector aSize, int32 PolygonSize, EQuadOptimizationType OptimizationType, EDownSamplingType DownSamplingType)
{
	if (!FMath::IsPowerOfTwo(aHeightMap->GetSizeX()) && !FMath::IsPowerOfTwo(aHeightMap->GetSizeY()))
	{
		FString lErrorReason = "Generate heightmap failed. ERROR: Texture is not power of two";

		if (GetWorld()->IsPlayInEditor())
		{
			FText lErrorMessage = FText::FromString(lErrorReason);
			if (FMessageDialog::Open(EAppMsgType::Ok, lErrorMessage) == EAppReturnType::Ok)
			{
				
			}
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *lErrorReason);
			}
			else
			{
				UE_LOG(LogClass, Log, TEXT("%s"), *lErrorReason);
			}
		}

		return;
	}

	Mesh->ClearAllMeshSections();

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;

	GenerateMesh(Vertices, Triangles, Normals, UVs, Tangents, VertexColors, aHeightMap, PolygonSize, OptimizationType, DownSamplingType);

	Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);
	Mesh->SetMaterial(0, aMat);

	FVector lMeshSize = Mesh->Bounds.GetBox().GetSize();	
	FVector lNewScale = FVector((aSize.X / lMeshSize.X), (aSize.Y / lMeshSize.Y), (aSize.Z / lMeshSize.Z));
	Mesh->SetWorldScale3D(lNewScale);

	Mesh->ShouldGenerateAutoLOD();

	UE_LOG(LogClass, Log, TEXT("Generated mesh: Verticles = %i | Triangles = %i | UVs = %i | Normals = %i | Tangents = %i"), Vertices.Num(), Triangles.Num(), UVs.Num(), Normals.Num(), Tangents.Num());
}

void ATestHeightMapLand::GenerateMesh(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents, TArray<FColor>& VertexColors, class UTexture2D* aHeightMap, int32 PolygonSize, EQuadOptimizationType OptimizationType, EDownSamplingType DownSamplingType)
{
	// 0. Init
	FIntPoint TextureSize = FIntPoint(aHeightMap->GetSizeX(), aHeightMap->GetSizeY());

	int32 Counter = int8(DownSamplingType);

	FIntPoint LandSize = TextureSize / Counter;

	// -----------------------------------------------------------------------------------------------------
	// 1. Add Normal Vertices and Normal Vertex Color  -  1 vertice per 1 pixel
	TArray<FVertice> NormalVertices;
	NormalVertices.Empty();

	FTexture2DMipMap* MyMipMap = &aHeightMap->PlatformData->Mips[0];
	FByteBulkData* RawImageData = &MyMipMap->BulkData;
	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));
	
	for (int32 x = 0; x < TextureSize.X; x = x + Counter)
	{
		for (int32 y = 0; y < TextureSize.Y; y = y + Counter)
		{
			FColor lPixelColor = FormatedImageData[y * TextureSize.X + x];

			float lHeight0 = lPixelColor.R;

			// Set up the vertices
			FVertice lVertice = FVertice();
			lVertice.Loc = FVector(x * PolygonSize, y * PolygonSize, lHeight0);
			lVertice.Color = lPixelColor;

			NormalVertices.Add(lVertice);
		}
	}

	RawImageData->Unlock();

	// -----------------------------------------------------------------------------------------------------
	// 2. Make quads (NormalVertices.Num() = NormalVertexColors. Num())
	TArray<FQuad> NormalQuads;
	NormalQuads.Empty();

	for (int32 i = 0; i < NormalVertices.Num() - LandSize.Y - 1; i++)
	{
		int32 CurrentX = i / LandSize.Y;
		int32 CurrentY = i % LandSize.Y;

		if (CurrentY == LandSize.Y - 1)
		{
			continue;
		}

		int VertIndex1 = i;
		int VertIndex2 = i + 1;
		int VertIndex3 = i + LandSize.Y + 1;
		int VertIndex4 = i + LandSize.Y;

		FQuad lQuad = FQuad();

		lQuad.V1 = VertIndex1;
		lQuad.V2 = VertIndex2;
		lQuad.V3 = VertIndex3;
		lQuad.V4 = VertIndex4;

		NormalQuads.Add(lQuad);
	}

	// -----------------------------------------------------------------------------------------------------
	// 3. Check quads height and save optimized quads
	TArray<FQuad> OptimizedQuads;
	OptimizedQuads.Empty();

	switch (OptimizationType)
	{
		case EQuadOptimizationType::None: OptimizedQuads = *&NormalQuads;
			break;
		case EQuadOptimizationType::AxesAlternately: OptimizeMeshQuads_AxesAlternately(NormalQuads, NormalVertices, LandSize, OptimizedQuads);
			break;
		case EQuadOptimizationType::Square: OptimizeMeshQuads_Square(NormalQuads, NormalVertices, LandSize, OptimizedQuads);
			break;
		case EQuadOptimizationType::Rectangle: OptimizeMeshQuads_Rectangle(NormalQuads, NormalVertices, LandSize, OptimizedQuads);
			break;
		case EQuadOptimizationType::FrontAndBack: OptimizeMeshQuads_FrontAndBack(NormalQuads, NormalVertices, LandSize, OptimizedQuads);
			break;
		default:
			break;
	}

	// -----------------------------------------------------------------------------------------------------
	// 4. Fill new optimized vertices with vertex colors
	Vertices.Reset();
	VertexColors.Reset();

	TArray<FQuad> OptimizedVerticlesQuad;
	OptimizedVerticlesQuad.Empty();

	for (int32 i = 0; i < OptimizedQuads.Num(); i++)
	{
		FQuad lOptimizedVerticleQuad = FQuad();

		lOptimizedVerticleQuad.V1 = Vertices.AddUnique(NormalVertices[OptimizedQuads[i].V1].Loc);
		if (Vertices[lOptimizedVerticleQuad.V1] == NormalVertices[OptimizedQuads[i].V1].Loc)
		{
			VertexColors.Add(NormalVertices[OptimizedQuads[i].V1].Color);
		}

		lOptimizedVerticleQuad.V2 = Vertices.AddUnique(NormalVertices[OptimizedQuads[i].V2].Loc);
		if (Vertices[lOptimizedVerticleQuad.V2] == NormalVertices[OptimizedQuads[i].V2].Loc)
		{
			VertexColors.Add(NormalVertices[OptimizedQuads[i].V2].Color);
		}

		lOptimizedVerticleQuad.V3 = Vertices.AddUnique(NormalVertices[OptimizedQuads[i].V3].Loc);
		if (Vertices[lOptimizedVerticleQuad.V3] == NormalVertices[OptimizedQuads[i].V3].Loc)
		{
			VertexColors.Add(NormalVertices[OptimizedQuads[i].V3].Color);
		}

		lOptimizedVerticleQuad.V4 = Vertices.AddUnique(NormalVertices[OptimizedQuads[i].V4].Loc);
		if (Vertices[lOptimizedVerticleQuad.V4] == NormalVertices[OptimizedQuads[i].V4].Loc)
		{
			VertexColors.Add(NormalVertices[OptimizedQuads[i].V4].Color);
		}

		OptimizedVerticlesQuad.Add(lOptimizedVerticleQuad);
	}


	// -----------------------------------------------------------------------------------------------------
	// 5. Make others:
	Triangles.Reset();

	int32 lVerticesQuantity = Vertices.Num();

	Normals.Reset();
	Normals.AddUninitialized(lVerticesQuantity);

	Tangents.Reset();
	Tangents.AddUninitialized(lVerticesQuantity);

	UVs.Reset();
	UVs.AddUninitialized(lVerticesQuantity);

	const FVector2D UVTextureScale = FVector2D(1.f / float(TextureSize.X), 1.f / float(TextureSize.Y));

	for (int32 i = 0; i < OptimizedVerticlesQuad.Num(); i++)
	{
		int VertIndex1 = OptimizedVerticlesQuad[i].V1;
		int VertIndex2 = OptimizedVerticlesQuad[i].V2;
		int VertIndex3 = OptimizedVerticlesQuad[i].V3;
		int VertIndex4 = OptimizedVerticlesQuad[i].V4;

		// Now create two triangles from those four vertices
		// The order of these (clockwise/counter-clockwise) dictates which way the normal will face. 
		Triangles.Add(VertIndex1);
		Triangles.Add(VertIndex2);
		Triangles.Add(VertIndex3);

		Triangles.Add(VertIndex1);
		Triangles.Add(VertIndex3);
		Triangles.Add(VertIndex4);

		//UVs
		UVs[VertIndex1] = FVector2D(UVTextureScale.X * Vertices[VertIndex1].X / float(PolygonSize), UVTextureScale.Y * Vertices[VertIndex1].Y / float(PolygonSize));
		UVs[VertIndex2] = FVector2D(UVTextureScale.X * Vertices[VertIndex2].X / float(PolygonSize), UVTextureScale.Y * Vertices[VertIndex2].Y / float(PolygonSize));
		UVs[VertIndex3] = FVector2D(UVTextureScale.X * Vertices[VertIndex3].X / float(PolygonSize), UVTextureScale.Y * Vertices[VertIndex3].Y / float(PolygonSize));
		UVs[VertIndex4] = FVector2D(UVTextureScale.X * Vertices[VertIndex4].X / float(PolygonSize), UVTextureScale.Y * Vertices[VertIndex4].Y / float(PolygonSize));

		// Normals
		Normals[VertIndex1] = FVector::CrossProduct(Vertices[VertIndex1] - Vertices[VertIndex4], Vertices[VertIndex1] - Vertices[VertIndex2]).GetSafeNormal();
		Normals[VertIndex2] = FVector::CrossProduct(Vertices[VertIndex2] - Vertices[VertIndex1], Vertices[VertIndex2] - Vertices[VertIndex3]).GetSafeNormal();
		Normals[VertIndex3] = FVector::CrossProduct(Vertices[VertIndex3] - Vertices[VertIndex2], Vertices[VertIndex3] - Vertices[VertIndex4]).GetSafeNormal();
		Normals[VertIndex4] = FVector::CrossProduct(Vertices[VertIndex4] - Vertices[VertIndex3], Vertices[VertIndex4] - Vertices[VertIndex1]).GetSafeNormal();

		// Tangents (perpendicular to the surface)
		Tangents[VertIndex1] = FProcMeshTangent((Vertices[VertIndex1] - Vertices[VertIndex3]).GetSafeNormal(), true);
		Tangents[VertIndex2] = FProcMeshTangent((Vertices[VertIndex2] - Vertices[VertIndex4]).GetSafeNormal(), true);
		Tangents[VertIndex3] = FProcMeshTangent((Vertices[VertIndex3] - Vertices[VertIndex1]).GetSafeNormal(), true);
		Tangents[VertIndex4] = FProcMeshTangent((Vertices[VertIndex4] - Vertices[VertIndex2]).GetSafeNormal(), true);
	}
}

void ATestHeightMapLand::OptimizeMeshQuads_AxesAlternately(TArray<FQuad>& NormalQuads, TArray<FVertice>& NormalVertices, FIntPoint& TextureSize, TArray<FQuad>& OptimizedQuads)
{
	int32 QuadColumns = TextureSize.X - 1;
	int32 QuadRows = TextureSize.Y - 1;

	TArray<FQuad> AxesQuads;
	AxesQuads.Empty();

	for (int32 x = 0; x < QuadColumns; x++)
	{
		for (int32 y = 0; y < QuadRows; y++)
		{
			int32 CurrentIdx = x * QuadRows + y;

			if (y > 0)
			{
				int32 Oidx = AxesQuads.Num() - 1;
				if (AxesQuads[Oidx].HasSameHeight(NormalQuads[CurrentIdx], NormalVertices))
				{
					FQuad NewQuad = FQuad();
					NewQuad.V1 = AxesQuads[Oidx].V1;
					NewQuad.V2 = NormalQuads[CurrentIdx].V2;
					NewQuad.V3 = NormalQuads[CurrentIdx].V3;
					NewQuad.V4 = AxesQuads[Oidx].V4;

					AxesQuads[Oidx] = NewQuad;

					continue;
				}
			}

			AxesQuads.Add(NormalQuads[CurrentIdx]);
		}
	}

	for (int32 i = 0; i < AxesQuads.Num(); i++)
	{
		if (i > 0)
		{
			int32 Oidx = OptimizedQuads.Num() - 1;
			if (OptimizedQuads[Oidx].HasSameHeight(AxesQuads[i], NormalVertices))
			{
				FQuad NewQuad = FQuad();
				NewQuad.V1 = OptimizedQuads[Oidx].V1;
				NewQuad.V2 = OptimizedQuads[Oidx].V2;
				NewQuad.V3 = AxesQuads[i].V3;
				NewQuad.V4 = AxesQuads[i].V4;

				OptimizedQuads[Oidx] = NewQuad;

				continue;
			}
		}

		OptimizedQuads.Add(AxesQuads[i]);
	}
}


void ATestHeightMapLand::OptimizeMeshQuads_FrontAndBack(TArray<FQuad>& NormalQuads, TArray<FVertice>& NormalVertices, FIntPoint& TextureSize, TArray<FQuad>& OptimizedQuads)
{
	int32 QuadColumns = TextureSize.X - 1;
	int32 QuadRows = TextureSize.Y - 1;

	int32 LastRowElements = 0;

	int32 OptimizedQuadCounter = 0;
	TArray<int32> inOptimizedIndexes;
	inOptimizedIndexes.Empty();

	for (int32 x = 0; x < QuadColumns; x++)
	{
		int32 RowCounter = 0;

		for (int32 y = 0; y < QuadRows; y++)
		{	
			int32 CurrentIdx = x * QuadRows + y;

			bool bContinueNext = false;

			if (y > 0)
			{
				int32 PrevIdx = OptimizedQuads.Num() - 1;
				if (OptimizedQuads[PrevIdx].HasSameHeight(NormalQuads[CurrentIdx], NormalVertices))
				{
					// combine with previous on Y
					FQuad NewQuad = FQuad();
					NewQuad.V1 = OptimizedQuads[PrevIdx].V1;
					NewQuad.V2 = NormalQuads[CurrentIdx].V2;
					NewQuad.V3 = NormalQuads[CurrentIdx].V3;
					NewQuad.V4 = OptimizedQuads[PrevIdx].V4;

					OptimizedQuads[PrevIdx] = NewQuad;

					if (y == (QuadRows - 1))
					{
						bContinueNext = true;					
					}
					else
					{
						continue;
					}
				}
			}

			if ((x > 0) && (LastRowElements > 0) && (OptimizedQuadCounter > LastRowElements))
			{
				int32 CurrentRowCounterIdx = OptimizedQuadCounter - 1;
				int32 PreviousRowCounterIdx = OptimizedQuadCounter - 1 - LastRowElements;

				int32 ThisRowIdx = inOptimizedIndexes[CurrentRowCounterIdx];
				int32 PrevRowIdx = inOptimizedIndexes[PreviousRowCounterIdx];

				if ((OptimizedQuads[PrevRowIdx].HasSameSide(OptimizedQuads[ThisRowIdx])) && (OptimizedQuads[PrevRowIdx].HasSameHeight(OptimizedQuads[ThisRowIdx], NormalVertices)))
				{
					// combine with previous column on X
					FQuad NewQuad = FQuad();
					NewQuad.V1 = OptimizedQuads[PrevRowIdx].V1;
					NewQuad.V2 = OptimizedQuads[PrevRowIdx].V2;
					NewQuad.V3 = OptimizedQuads[ThisRowIdx].V3;
					NewQuad.V4 = OptimizedQuads[ThisRowIdx].V4;

					OptimizedQuads[PrevRowIdx] = NewQuad;

					OptimizedQuads.RemoveAt(ThisRowIdx);

					inOptimizedIndexes[CurrentRowCounterIdx] = inOptimizedIndexes[PreviousRowCounterIdx];
				}
			}
			
			if (bContinueNext)
			{
				continue;
			}
			else
			{
				RowCounter++;

				OptimizedQuadCounter++;

				int32 NewIndex = OptimizedQuads.Add(NormalQuads[CurrentIdx]);

				inOptimizedIndexes.Add(NewIndex);
			}
		}

		LastRowElements = RowCounter;
	}
}


void ATestHeightMapLand::OptimizeMeshQuads_Square(TArray<FQuad>& NormalQuads, TArray<FVertice>& NormalVertices, FIntPoint& TextureSize, TArray<FQuad>& OptimizedQuads)
{
	int32 QuadColumns = TextureSize.X - 1;
	int32 QuadRows = TextureSize.Y - 1;

	for (int32 i = 0; i < NormalQuads.Num(); i++)
	{
		if (NormalQuads[i].bUsed)
		{
			continue;
		}
		else
		{
			NormalQuads[i].bUsed = true;
		}

		int32 CurrentX = i / TextureSize.Y;
		int32 CurrentY = i % TextureSize.Y;

		TArray<FQuad> OptimalQuads;
		OptimalQuads.Empty();

		OptimalQuads.Add(NormalQuads[i]); // 1st element is current [i]

		int32 NextX = CurrentX;
		int32 NextY = CurrentY;

		bool bOptimizing = true;
		while (bOptimizing)
		{
			NextX++;
			if (NextX > QuadColumns)
			{
				bOptimizing = false;
			}

			NextY++;
			if (NextY > QuadRows)
			{
				bOptimizing = false;
			}

			if (bOptimizing)
			{
				// по Х
				TArray<FQuad> XAxesQuads;
				XAxesQuads.Empty();

				for (int32 x = CurrentX; x <= NextX - 1; x++)
				{
					int32 TempIdx = x * QuadRows + NextY;
					if (NormalQuads[TempIdx].bUsed || !NormalQuads[TempIdx].HasSameHeight(NormalQuads[i], NormalVertices))
					{
						XAxesQuads.Empty();
						bOptimizing = false;
						break;
					}
					else
					{
						XAxesQuads.Add(NormalQuads[TempIdx]);
					}
				}

				if (XAxesQuads.Num() > 0)
				{
					for (int32 k = 0; k < XAxesQuads.Num(); k++)
					{
						XAxesQuads[k].bUsed = true;
						OptimalQuads.Add(XAxesQuads[k]);
					}
				}
			}

			if (bOptimizing)
			{
				// по Y
				TArray<FQuad> YAxesQuads;
				YAxesQuads.Empty();

				for (int32 y = NextY; y >= CurrentY; y--)
				{
					int32 TempIdx = NextX * QuadRows + y;
					if (NormalQuads[TempIdx].bUsed || !NormalQuads[TempIdx].HasSameHeight(NormalQuads[i], NormalVertices))
					{
						YAxesQuads.Empty();
						bOptimizing = false;
						break;
					}
					else
					{
						YAxesQuads.Add(NormalQuads[TempIdx]);
					}
				}

				if (YAxesQuads.Num() > 0)
				{
					for (int32 k = 0; k < YAxesQuads.Num(); k++)
					{
						YAxesQuads[k].bUsed = true;
						OptimalQuads.Add(YAxesQuads[k]);
					}
				}
			}
			
			if (!bOptimizing)
			{
				int32 Kidx = int32(FMath::Sqrt(OptimalQuads.Num()));
				int32 i4 = OptimalQuads.Num() - 1;
				int32 i3 = i4 - (Kidx - 1);
				int32 i2 = i3 - (Kidx - 1);
				int32 i1 = 0;

				FQuad lNewQuad = FQuad();

				lNewQuad.V1 = OptimalQuads[i1].V1;
				lNewQuad.V2 = OptimalQuads[i2].V2;
				lNewQuad.V3 = OptimalQuads[i3].V3;
				lNewQuad.V4 = OptimalQuads[i4].V4;

				OptimizedQuads.Add(lNewQuad);
			}
		}
	}
}

void ATestHeightMapLand::OptimizeMeshQuads_Rectangle(TArray<FQuad>& NormalQuads, TArray<FVertice>& NormalVertices, FIntPoint& TextureSize, TArray<FQuad>& OptimizedQuads)
{
	int32 QuadColumns = TextureSize.X - 1;
	int32 QuadRows = TextureSize.Y - 1;

	for (int32 i = 0; i < NormalQuads.Num(); i++)
	{
		if (NormalQuads[i].bUsed)
		{
			continue;
		}
		else
		{
			NormalQuads[i].bUsed = true;
		}

		int32 CurrentX = i / TextureSize.Y;
		int32 CurrentY = i % TextureSize.Y;

		TArray<FQuad> OptimalQuads;
		OptimalQuads.Empty();

		OptimalQuads.Add(NormalQuads[i]); // 1st element is current [i]

		// Corners indexes:
		int32 i1 = 0;
		int32 i2 = 0;
		int32 i3 = 0;
		int32 i4 = 0;

		int32 NextX = CurrentX;
		int32 NextY = CurrentY;

		bool bHorizontal = true;
		bool bVertical = true;
		bool bXDirection = true;
		bool bYDirection = true;

		bool bLoop = true;
		while (bLoop)
		{
			if (bXDirection)
			{
				int32 TestNextX = NextX + 1;

				if (TestNextX > QuadColumns)
				{
					bXDirection = false;
					bVertical = false;
				}
				else
				{
					NextX = TestNextX;
				}
			}

			if (bYDirection)
			{
				int32 TestNextY = NextY + 1;

				if (TestNextY > QuadRows)
				{
					bYDirection = false;
					bHorizontal = false;
				}
				else
				{
					NextY = TestNextY;
				}
			}

			if (bHorizontal)
			{
				TArray<FQuad> XAxesQuads;
				XAxesQuads.Empty();

				for (int32 x = CurrentX; x <= NextX - 1; x++)
				{
					int32 TempIdx = x * QuadRows + NextY;
					if (NormalQuads[TempIdx].bUsed || !NormalQuads[TempIdx].HasSameHeight(NormalQuads[i], NormalVertices))
					{
						XAxesQuads.Empty();
						bYDirection = false;
						bHorizontal = false;
						NextY--;
						break;
					}
					else
					{
						XAxesQuads.Add(NormalQuads[TempIdx]);
					}
				}

				if (XAxesQuads.Num() > 0)
				{
					for (int32 k = 0; k < XAxesQuads.Num(); k++)
					{
						XAxesQuads[k].bUsed = true;
						OptimalQuads.Add(XAxesQuads[k]);
					}

					i2 = OptimalQuads.Num() - XAxesQuads.Num();

					if (!bVertical)
					{
						i3 = OptimalQuads.Num() - 1;
					}
				}
			}

			if (bVertical)
			{
				TArray<FQuad> YAxesQuads;
				YAxesQuads.Empty();

				for (int32 y = NextY; y >= CurrentY; y--)
				{
					int32 TempIdx = NextX * QuadRows + y;
					if (NormalQuads[TempIdx].bUsed || !NormalQuads[TempIdx].HasSameHeight(NormalQuads[i], NormalVertices))
					{
						YAxesQuads.Empty();
						bXDirection = false;
						bVertical = false;
						NextX--;
						break;
					}
					else
					{
						YAxesQuads.Add(NormalQuads[TempIdx]);
					}
				}

				if (YAxesQuads.Num() > 0)
				{
					for (int32 k = 0; k < YAxesQuads.Num(); k++)
					{
						YAxesQuads[k].bUsed = true;
						OptimalQuads.Add(YAxesQuads[k]);
					}

					if (!bHorizontal)
					{
						i3 = OptimalQuads.Num() - YAxesQuads.Num();
					}

					i4 = OptimalQuads.Num() - 1;
				}
			}

			// should stop
			if (!bHorizontal && !bVertical)
			{
				FQuad lNewQuad = FQuad();
				lNewQuad.V1 = OptimalQuads[i1].V1;
				lNewQuad.V2 = OptimalQuads[i2].V2;
				lNewQuad.V3 = OptimalQuads[i3].V3;
				lNewQuad.V4 = OptimalQuads[i4].V4;

				OptimizedQuads.Add(lNewQuad);

				bLoop = false;

				break;
			}
		}
	}
}