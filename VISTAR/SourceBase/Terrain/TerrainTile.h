// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "TerrainConfig.h"
#include "TerrainTile.generated.h"

/**
 * Individual terrain tile actor
 * Represents a single tile in the terrain grid system
 */
UCLASS()
class VISTAR_API ATerrainTile : public AActor
{
    GENERATED_BODY()

public:
    ATerrainTile();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Initialize tile with given parameters
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void InitializeTile(int32 InTileX, int32 InTileY, const FTerrainConfig& InConfig);

    // Generate mesh for this tile
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void GenerateMesh();

    // Update LOD based on distance to camera
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void UpdateLOD(FVector CameraLocation);

    // Get tile coordinates
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FIntPoint GetTileCoordinates() const { return FIntPoint(TileX, TileY); }

    // Get world position of tile center
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FVector GetTileCenter() const;

    // Get height at a world position
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetHeightAtLocation(FVector WorldLocation) const;

    // Check if tile contains a world position
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    bool ContainsWorldLocation(FVector WorldLocation) const;

    // Set material
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void SetTerrainMaterial(UMaterialInterface* Material);

protected:
    // Procedural mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProceduralMeshComponent* ProceduralMesh;

    // Tile identification
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
    int32 TileX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
    int32 TileY;

    // Current LOD level
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
    int32 CurrentLOD;

    // Cached configuration
    UPROPERTY()
    FTerrainConfig Config;

    // Cached height data for this tile
    UPROPERTY()
    TArray<float> HeightData;

    // Is the tile initialized
    UPROPERTY()
    bool bIsInitialized;

private:
    // Generate height value using Perlin noise
    float GenerateHeight(float WorldX, float WorldY) const;

    // Perlin noise helper
    float PerlinNoise2D(float X, float Y, int32 Seed) const;

    // Generate mesh vertices and triangles
    void GenerateMeshData(int32 LODLevel, TArray<FVector>& Vertices, TArray<int32>& Triangles,
                          TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FColor>& VertexColors);

    // Calculate normal for a vertex
    FVector CalculateNormal(int32 X, int32 Y, int32 Resolution) const;
};
