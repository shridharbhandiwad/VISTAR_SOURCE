// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * Terrain System Header
 * 
 * Include this header to access all terrain-related classes:
 * - FTerrainConfig / FFoliageConfig - Configuration structs
 * - ATerrainManager - Main terrain generation and management
 * - ATerrainTile - Individual terrain tile actor
 * - AFoliageManager - Foliage/tree spawning and management
 * 
 * USAGE EXAMPLE (in Blueprint or C++):
 * 
 * 1. Spawn ATerrainManager in your level
 * 2. Configure TerrainConfig (50km x 50km by default):
 *    - TotalSizeX/Y = 5,000,000 (50km in cm)
 *    - TileSizeX/Y = 100,000 (1km tiles)
 *    - TileResolution = 64 (vertices per tile edge)
 * 
 * 3. Spawn AFoliageManager and link to TerrainManager
 * 4. Configure foliage types with meshes
 * 
 * 5. Call InitializeTerrain() then GenerateTilesAroundPosition()
 *    or GenerateAllTiles() for full terrain generation
 * 
 * 6. Call InitializeFoliage() then GenerateFoliageInArea()
 *    or GenerateAllFoliage() for tree generation
 * 
 * PERFORMANCE NOTES:
 * - 50km x 50km = 2,500 km² = 2,500 tiles (1km each)
 * - Generation is async and throttled to avoid hitching
 * - Streaming automatically loads/unloads tiles based on camera distance
 * - LOD reduces detail for distant tiles
 * - Trees use HISM (Hierarchical Instanced Static Mesh) for efficiency
 */

#include "TerrainConfig.h"
#include "TerrainTile.h"
#include "TerrainManager.h"
#include "FoliageManager.h"
