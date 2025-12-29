// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainConfig.h"
#include "TerrainTile.h"
#include "TerrainManager.generated.h"

/**
 * Manager class for generating and managing large-scale terrain (50km x 50km)
 * Handles tile streaming, LOD management, and terrain queries
 */
UCLASS()
class VISTAR_API ATerrainManager : public AActor
{
    GENERATED_BODY()

public:
    ATerrainManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FTerrainConfig TerrainConfig;

    // Initialize terrain system
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void InitializeTerrain();

    // Generate all terrain tiles (can be slow for 50x50km!)
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void GenerateAllTiles();

    // Generate tiles in a radius around a position
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void GenerateTilesAroundPosition(FVector Position, float Radius);

    // Update streaming - load/unload tiles based on camera position
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void UpdateStreaming(FVector CameraPosition);

    // Get terrain height at a world position
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetHeightAtLocation(FVector WorldLocation) const;

    // Get the tile at a world position
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    ATerrainTile* GetTileAtLocation(FVector WorldLocation) const;

    // Get tile by coordinates
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    ATerrainTile* GetTile(int32 TileX, int32 TileY) const;

    // Check if terrain has been initialized
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    bool IsInitialized() const { return bIsInitialized; }

    // Get terrain bounds
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FBox GetTerrainBounds() const;

    // Delegate for when terrain generation is complete
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTerrainGenerationComplete);
    UPROPERTY(BlueprintAssignable, Category = "Terrain")
    FOnTerrainGenerationComplete OnTerrainGenerationComplete;

    // Delegate for progress updates during generation
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerrainGenerationProgress, int32, TilesGenerated, int32, TotalTiles);
    UPROPERTY(BlueprintAssignable, Category = "Terrain")
    FOnTerrainGenerationProgress OnTerrainGenerationProgress;

protected:
    // All terrain tiles (sparse - only created when needed)
    UPROPERTY()
    TMap<FIntPoint, ATerrainTile*> TerrainTiles;

    // Currently loaded tiles
    UPROPERTY()
    TSet<FIntPoint> LoadedTiles;

    // Initialization state
    UPROPERTY()
    bool bIsInitialized;

    // Generation state (for async generation)
    UPROPERTY()
    bool bIsGenerating;

    UPROPERTY()
    int32 CurrentGenerationIndex;

    UPROPERTY()
    TArray<FIntPoint> TilesToGenerate;

    // Time tracking for throttled generation
    float GenerationTimeAccumulator;
    float MaxGenerationTimePerFrame;

private:
    // Create a single tile
    ATerrainTile* CreateTile(int32 TileX, int32 TileY);

    // Convert world position to tile coordinates
    FIntPoint WorldToTileCoords(FVector WorldLocation) const;

    // Check if tile coordinates are valid
    bool AreTileCoordsValid(int32 TileX, int32 TileY) const;

    // Get tiles within radius of a point
    TArray<FIntPoint> GetTilesInRadius(FVector Center, float Radius) const;

    // Process async tile generation
    void ProcessAsyncGeneration(float DeltaTime);
};
