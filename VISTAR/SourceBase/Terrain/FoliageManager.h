// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "TerrainConfig.h"
#include "TerrainManager.h"
#include "FoliageManager.generated.h"

/**
 * Foliage type definition
 */
USTRUCT(BlueprintType)
struct VISTAR_API FFoliageType
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    FString TypeName = "Tree";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    UStaticMesh* Mesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float DensityPerKm2 = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float MinScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float MaxScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float MinSpawnHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float MaxSpawnHeight = 40000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float MaxSpawnSlope = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float CullDistance = 1000000.0f; // 10km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    bool bAlignToSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    bool bRandomYaw = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    float RandomPitchRange = 5.0f; // degrees
};

/**
 * Manager class for spawning and managing foliage (trees, bushes, grass) across terrain
 * Uses Hierarchical Instanced Static Mesh for efficient rendering
 */
UCLASS()
class VISTAR_API AFoliageManager : public AActor
{
    GENERATED_BODY()

public:
    AFoliageManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    FFoliageConfig FoliageConfig;

    // Foliage types to spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    TArray<FFoliageType> FoliageTypes;

    // Reference to terrain manager
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
    ATerrainManager* TerrainManager;

    // Initialize foliage system
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void InitializeFoliage();

    // Generate foliage for entire terrain (can be very slow!)
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void GenerateAllFoliage();

    // Generate foliage in a specific area
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void GenerateFoliageInArea(FVector Center, float Radius);

    // Generate foliage for a specific terrain tile
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void GenerateFoliageForTile(int32 TileX, int32 TileY);

    // Clear all foliage
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void ClearAllFoliage();

    // Clear foliage in an area
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void ClearFoliageInArea(FVector Center, float Radius);

    // Get total foliage instance count
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    int32 GetTotalInstanceCount() const;

    // Update streaming - hide/show foliage based on camera distance
    UFUNCTION(BlueprintCallable, Category = "Foliage")
    void UpdateStreaming(FVector CameraPosition);

    // Delegate for generation progress
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFoliageGenerationProgress, int32, TilesProcessed, int32, TotalTiles);
    UPROPERTY(BlueprintAssignable, Category = "Foliage")
    FOnFoliageGenerationProgress OnFoliageGenerationProgress;

    // Delegate for generation complete
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFoliageGenerationComplete, int32, TotalInstances);
    UPROPERTY(BlueprintAssignable, Category = "Foliage")
    FOnFoliageGenerationComplete OnFoliageGenerationComplete;

protected:
    // HISM components for each foliage type
    UPROPERTY()
    TMap<FString, UHierarchicalInstancedStaticMeshComponent*> HISMComponents;

    // Track which tiles have foliage generated
    UPROPERTY()
    TSet<FIntPoint> GeneratedTiles;

    // Initialization state
    UPROPERTY()
    bool bIsInitialized;

    // Async generation state
    UPROPERTY()
    bool bIsGenerating;

    UPROPERTY()
    int32 CurrentGenerationTileIndex;

    UPROPERTY()
    TArray<FIntPoint> TilesToProcess;

    float GenerationTimeAccumulator;
    float MaxGenerationTimePerFrame;

private:
    // Create HISM component for a foliage type
    UHierarchicalInstancedStaticMeshComponent* CreateHISMComponent(const FFoliageType& FoliageType);

    // Spawn foliage instances for a tile
    void SpawnFoliageForTile(int32 TileX, int32 TileY);

    // Calculate spawn positions using Poisson disk sampling
    TArray<FVector2D> GenerateSpawnPositions(float AreaSizeX, float AreaSizeY, float MinDistance, int32 Seed);

    // Simple Poisson disk sampling
    TArray<FVector2D> PoissonDiskSampling(float Width, float Height, float MinDistance, int32 Seed, int32 MaxAttempts = 30);

    // Get surface normal at position (from terrain)
    FVector GetSurfaceNormal(FVector Position) const;

    // Process async generation
    void ProcessAsyncGeneration(float DeltaTime);

    // Random stream for deterministic generation
    FRandomStream RandomStream;
};
