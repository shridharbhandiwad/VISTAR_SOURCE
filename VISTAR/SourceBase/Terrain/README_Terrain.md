# VISTAR Terrain & Foliage System

## Overview

This terrain system provides procedural generation of large-scale terrain (50km × 50km by default) with automatic LOD, streaming, and foliage placement.

## Components

### TerrainConfig.h
Configuration structures for terrain and foliage:
- `FTerrainConfig` - Terrain generation settings (size, tile resolution, noise parameters, LOD distances)
- `FFoliageConfig` - Foliage density, scale ranges, spawn constraints

### TerrainManager
Main controller for terrain generation and management:
- Generates procedural terrain tiles using Perlin noise
- Handles tile streaming (load/unload based on camera distance)
- Manages LOD levels for distant terrain
- Provides height queries at any world position

### TerrainTile
Individual terrain tile actor:
- Uses ProceduralMeshComponent for runtime mesh generation
- Supports multiple LOD levels
- Provides height interpolation for precise positioning

### FoliageManager
Vegetation spawning system:
- Uses Hierarchical Instanced Static Mesh (HISM) for efficient rendering
- Poisson disk sampling for natural distribution
- Supports multiple foliage types (trees, bushes, grass)
- Automatic culling at distance

## Default Configuration (50km × 50km)

```
Terrain:
- Total Size: 50km × 50km (5,000,000 × 5,000,000 Unreal units)
- Tile Size: 1km × 1km (100,000 × 100,000 units)
- Total Tiles: 50 × 50 = 2,500 tiles
- Tile Resolution: 64 vertices per edge
- Max Height: 500m variation
- Streaming Distance: 15km
- LOD Distances: 5km (full), 10km (medium), 20km (low)

Foliage:
- Tree Density: 500 per km² (default)
- Bush Density: 1,000 per km² (default)
- Culling Distance: 10km for trees, 5km for bushes
```

## Usage

### Blueprint Setup

1. **Place Terrain Manager:**
   - Drag `ATerrainManager` into your level
   - Configure `TerrainConfig` in Details panel
   - Set terrain material if desired

2. **Place Foliage Manager:**
   - Drag `AFoliageManager` into your level
   - Set `TerrainManager` reference
   - Add foliage types with static meshes

3. **Initialize on BeginPlay:**
   ```
   TerrainManager -> InitializeTerrain()
   TerrainManager -> GenerateTilesAroundPosition(PlayerStart, 5000000)  // 5km radius
   
   FoliageManager -> InitializeFoliage()
   FoliageManager -> GenerateFoliageInArea(PlayerStart, 5000000)
   ```

### C++ Setup

```cpp
#include "SourceBase/Terrain/Terrain.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Spawn terrain manager
    FActorSpawnParameters SpawnParams;
    ATerrainManager* TerrainMgr = GetWorld()->SpawnActor<ATerrainManager>(
        ATerrainManager::StaticClass(), 
        FVector::ZeroVector, 
        FRotator::ZeroRotator, 
        SpawnParams
    );
    
    // Configure for 50km x 50km (default)
    TerrainMgr->TerrainConfig.TotalSizeX = 5000000.0f;
    TerrainMgr->TerrainConfig.TotalSizeY = 5000000.0f;
    
    // Initialize and generate
    TerrainMgr->InitializeTerrain();
    
    // Generate terrain around player spawn
    FVector PlayerStart = GetPlayerStart()->GetActorLocation();
    TerrainMgr->GenerateTilesAroundPosition(PlayerStart, 1000000.0f); // 10km radius
    
    // Setup foliage
    AFoliageManager* FoliageMgr = GetWorld()->SpawnActor<AFoliageManager>(
        AFoliageManager::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );
    FoliageMgr->TerrainManager = TerrainMgr;
    
    // Add tree type
    FFoliageType TreeType;
    TreeType.TypeName = "Pine";
    TreeType.Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/PineTree"));
    TreeType.DensityPerKm2 = 500.0f;
    FoliageMgr->FoliageTypes.Add(TreeType);
    
    FoliageMgr->InitializeFoliage();
    FoliageMgr->GenerateFoliageInArea(PlayerStart, 1000000.0f);
}
```

## Performance Tips

1. **Don't generate all tiles at once** - Use `GenerateTilesAroundPosition()` with streaming
2. **Adjust LOD distances** - Reduce for better performance on lower-end hardware
3. **Use appropriate foliage density** - 500 trees/km² is reasonable; reduce for large areas
4. **Set culling distances** - Trees don't need to render at 20km
5. **Reduce tile resolution** - 32 or even 16 vertices is often sufficient for distant terrain

## Memory Estimates (50km × 50km)

- **Terrain tiles (all loaded):** ~2,500 tiles × ~32KB = ~80MB
- **Trees (500/km² full area):** ~1,250,000 instances × ~64 bytes = ~80MB
- **With streaming (10km radius):** Much less, as only ~300 tiles loaded

## Events

Both managers broadcast events for progress tracking:
- `OnTerrainGenerationProgress(TilesGenerated, TotalTiles)`
- `OnTerrainGenerationComplete()`
- `OnFoliageGenerationProgress(TilesProcessed, TotalTiles)`
- `OnFoliageGenerationComplete(TotalInstances)`

## Customization

### Terrain Shape
Modify noise parameters in `FTerrainConfig`:
- `NoiseScale` - Larger = smoother terrain
- `NoiseOctaves` - More = more detail layers
- `NoisePersistence` - Higher = rougher terrain
- `NoiseLacunarity` - Higher = more high-frequency detail

### Custom Heights
Override `ATerrainTile::GenerateHeight()` for custom terrain shapes:
- Import real heightmap data
- Create specific terrain features
- Blend multiple noise functions

### Custom Foliage Placement
Override `AFoliageManager::SpawnFoliageForTile()` for:
- Biome-based vegetation
- Road/path avoidance
- Building exclusion zones
