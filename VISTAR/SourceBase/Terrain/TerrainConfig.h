// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainConfig.generated.h"

/**
 * Configuration structure for terrain generation
 * Default configuration generates a 50km x 50km terrain
 */
USTRUCT(BlueprintType)
struct VISTAR_API FTerrainConfig
{
    GENERATED_BODY()

    // Total terrain size in centimeters (Unreal units)
    // 50km = 50,000m = 5,000,000cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Size")
    float TotalSizeX = 5000000.0f; // 50km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Size")
    float TotalSizeY = 5000000.0f; // 50km

    // Size of each terrain tile in centimeters
    // 1km = 1000m = 100,000cm per tile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Tiles")
    float TileSizeX = 100000.0f; // 1km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Tiles")
    float TileSizeY = 100000.0f; // 1km

    // Number of vertices per tile edge (higher = more detail, more performance cost)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Detail", meta = (ClampMin = "2", ClampMax = "256"))
    int32 TileResolution = 64;

    // Height variation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Height")
    float MaxHeight = 50000.0f; // 500m max height variation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Height")
    float MinHeight = 0.0f;

    // Perlin noise parameters for terrain generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float NoiseScale = 0.00001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    int32 NoiseOctaves = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float NoisePersistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float NoiseLacunarity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    int32 NoiseSeed = 12345;

    // LOD settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|LOD")
    float LOD0Distance = 500000.0f; // 5km - full detail

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|LOD")
    float LOD1Distance = 1000000.0f; // 10km - medium detail

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|LOD")
    float LOD2Distance = 2000000.0f; // 20km - low detail

    // Streaming settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Streaming")
    float StreamingDistance = 1500000.0f; // 15km - tiles beyond this are unloaded

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Streaming")
    bool bEnableStreaming = true;

    // Material settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Material")
    UMaterialInterface* TerrainMaterial = nullptr;

    // Helper function to get number of tiles
    int32 GetNumTilesX() const { return FMath::CeilToInt(TotalSizeX / TileSizeX); }
    int32 GetNumTilesY() const { return FMath::CeilToInt(TotalSizeY / TileSizeY); }
    int32 GetTotalNumTiles() const { return GetNumTilesX() * GetNumTilesY(); }
};

/**
 * Configuration structure for foliage/tree generation
 */
USTRUCT(BlueprintType)
struct VISTAR_API FFoliageConfig
{
    GENERATED_BODY()

    // Foliage density (instances per square kilometer)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Density")
    float TreeDensityPerKm2 = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Density")
    float BushDensityPerKm2 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Density")
    float GrassDensityPerKm2 = 5000.0f;

    // Scale variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Scale")
    float MinTreeScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Scale")
    float MaxTreeScale = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Scale")
    float MinBushScale = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Scale")
    float MaxBushScale = 1.2f;

    // Height constraints (don't spawn trees above/below certain heights)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Placement")
    float MinSpawnHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Placement")
    float MaxSpawnHeight = 40000.0f; // 400m - no trees above this

    // Slope constraints (radians)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Placement")
    float MaxSpawnSlope = 0.7f; // ~40 degrees

    // Random rotation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Placement")
    bool bRandomYawRotation = true;

    // Culling distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|LOD")
    float TreeCullDistance = 1000000.0f; // 10km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|LOD")
    float BushCullDistance = 500000.0f; // 5km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|LOD")
    float GrassCullDistance = 100000.0f; // 1km

    // Seed for procedural generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Generation")
    int32 FoliageSeed = 54321;

    // Meshes to use
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Meshes")
    TArray<UStaticMesh*> TreeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Meshes")
    TArray<UStaticMesh*> BushMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Meshes")
    TArray<UStaticMesh*> GrassMeshes;
};
