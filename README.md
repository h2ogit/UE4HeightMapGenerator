# UE4HeightMapGenerator

Example how to generate mesh from heightmap texture using UProceduralMeshComponent.

You can define:

1. Texture to generate from (shoud be grayscale and power of two)

2. Material assign to generated mesh

3. Generated mesh size

4. Polygon size

5. Optimization type:

		None, // without optimization
		AxesAlternately, // 2 pass: first Y then X
		FrontAndBack, // 1 pass: front on Y and back on X
	
6. Downsampling type

		None = 1, // Same as original
		TwoTimes = 2, // 2 times lower
		FourTimes = 4, // 4 times lower


Class ATestHeightMapLand can be spawned and used in any project, just add .cpp and .h files

Bugs:

		1. Rectangle and Square optimizations are not finished still. So do not use them.

		2. Optimized mesh has wrong normals

![UE4HeightMapGenerator](https://github.com/h2ogit/UE4HeightMapGenerator/blob/master/1.png)
![UE4HeightMapGenerator](https://github.com/h2ogit/UE4HeightMapGenerator/blob/master/2.png)
![UE4HeightMapGenerator](https://github.com/h2ogit/UE4HeightMapGenerator/blob/master/3.png)
