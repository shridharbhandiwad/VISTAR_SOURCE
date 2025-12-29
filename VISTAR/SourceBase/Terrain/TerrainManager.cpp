// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

ATerrainManager::ATerrainManager()
{
    PrimaryActorTick.bCanEverTick = true;

    bIsInitialized = false;
    bIsGenerating = false;
    CurrentGenerationIndex = 0;
    GenerationTimeAccumulator = 0.0f;
    MaxGenerationTimePerFrame = 0.033f; // ~30ms per frame for generation
}

void ATerrainManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("TerrainManager: BeginPlay - Starting terrain system"));
    UE_LOG(LogTemp, Log, TEXT("TerrainManager: Terrain size = %.0f x %.0f (%.2f km x %.2f km)"),
           TerrainConfig.TotalSizeX, TerrainConfig.TotalSizeY,
           TerrainConfig.TotalSizeX / 100000.0f, TerrainConfig.TotalSizeY / 100000.0f);
    UE_LOG(LogTemp, Log, TEXT("TerrainManager: Tile count = %d x %d = %d tiles"),
           TerrainConfig.GetNumTilesX(), TerrainConfig.GetNumTilesY(), TerrainConfig.GetTotalNumTiles());
}

void ATerrainManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all tiles
    for (auto& Pair : TerrainTiles)
    {
        if (Pair.Value && IsValid(Pair.Value))
        {
            Pair.Value->Destroy();
        }
    }
    TerrainTiles.Empty();
    LoadedTiles.Empty();

    Super::EndPlay(EndPlayReason);
}

void ATerrainManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Process async generation if in progress
    if (bIsGenerating)
    {
        ProcessAsyncGeneration(DeltaTime);
    }

    // Update streaming based on camera position
    if (bIsInitialized && TerrainConfig.bEnableStreaming)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
            if (CameraManager)
            {
                FVector CameraLocation = CameraManager->GetCameraLocation();
                UpdateStreaming(CameraLocation);

                // Update LOD for loaded tiles
                for (const FIntPoint& TileCoord : LoadedTiles)
                {
                    ATerrainTile* Tile = TerrainTiles.FindRef(TileCoord);
                    if (Tile)
                    {
                        Tile->UpdateLOD(CameraLocation);
                    }
                }
            }
        }
    }
}

void ATerrainManager::InitializeTerrain()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainManager: Terrain already initialized"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("TerrainManager: Initializing terrain..."));

    bIsInitialized = true;

    // Don't auto-generate all tiles - use streaming or explicit generation
    UE_LOG(LogTemp, Log, TEXT("TerrainManager: Terrain system initialized. Call GenerateAllTiles() or GenerateTilesAroundPosition() to create terrain."));
}

void ATerrainManager::GenerateAllTiles()
{
    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainManager: Already generating tiles"));
        return;
    }

    if (!bIsInitialized)
    {
        InitializeTerrain();
    }

    UE_LOG(LogTemp, Log, TEXT("TerrainManager: Starting generation of all %d tiles (this may take a while)..."),
           TerrainConfig.GetTotalNumTiles());

    // Queue all tiles for async generation
    TilesToGenerate.Empty();
    for (int32 Y = 0; Y < TerrainConfig.GetNumTilesY(); Y++)
    {
        for (int32 X = 0; X < TerrainConfig.GetNumTilesX(); X++)
        {
            TilesToGenerate.Add(FIntPoint(X, Y));
        }
    }

    CurrentGenerationIndex = 0;
    bIsGenerating = true;
}

void ATerrainManager::GenerateTilesAroundPosition(FVector Position, float Radius)
{
    if (!bIsInitialized)
    {
        InitializeTerrain();
    }

    TArray<FIntPoint> TilesToCreate = GetTilesInRadius(Position, Radius);

    int32 CreatedCount = 0;
    for (const FIntPoint& TileCoord : TilesToCreate)
    {
        if (!TerrainTiles.Contains(TileCoord))
        {
            ATerrainTile* NewTile = CreateTile(TileCoord.X, TileCoord.Y);
            if (NewTile)
            {
                TerrainTiles.Add(TileCoord, NewTile);
                LoadedTiles.Add(TileCoord);
                CreatedCount++;
            }
        }
        else if (!LoadedTiles.Contains(TileCoord))
        {
            LoadedTiles.Add(TileCoord);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TerrainManager: Generated %d tiles around position (%.0f, %.0f)"),
           CreatedCount, Position.X, Position.Y);
}

void ATerrainManager::UpdateStreaming(FVector CameraPosition)
{
    if (!bIsInitialized)
    {
        return;
    }

    // Get tiles that should be loaded
    TArray<FIntPoint> ShouldBeLoaded = GetTilesInRadius(CameraPosition, TerrainConfig.StreamingDistance);
    TSet<FIntPoint> ShouldBeLoadedSet(ShouldBeLoaded);

    // Unload tiles that are too far
    TArray<FIntPoint> ToUnload;
    for (const FIntPoint& TileCoord : LoadedTiles)
    {
        if (!ShouldBeLoadedSet.Contains(TileCoord))
        {
            ToUnload.Add(TileCoord);
        }
    }

    for (const FIntPoint& TileCoord : ToUnload)
    {
        LoadedTiles.Remove(TileCoord);
        ATerrainTile* Tile = TerrainTiles.FindRef(TileCoord);
        if (Tile)
        {
            Tile->SetActorHiddenInGame(true);
            Tile->SetActorEnableCollision(false);
        }
    }

    // Load tiles that should be visible
    for (const FIntPoint& TileCoord : ShouldBeLoaded)
    {
        if (!TerrainTiles.Contains(TileCoord))
        {
            // Create new tile
            ATerrainTile* NewTile = CreateTile(TileCoord.X, TileCoord.Y);
            if (NewTile)
            {
                TerrainTiles.Add(TileCoord, NewTile);
                LoadedTiles.Add(TileCoord);
            }
        }
        else if (!LoadedTiles.Contains(TileCoord))
        {
            // Re-enable existing tile
            ATerrainTile* Tile = TerrainTiles.FindRef(TileCoord);
            if (Tile)
            {
                Tile->SetActorHiddenInGame(false);
                Tile->SetActorEnableCollision(true);
                LoadedTiles.Add(TileCoord);
            }
        }
    }
}

float ATerrainManager::GetHeightAtLocation(FVector WorldLocation) const
{
    ATerrainTile* Tile = GetTileAtLocation(WorldLocation);
    if (Tile)
    {
        return Tile->GetHeightAtLocation(WorldLocation);
    }
    return 0.0f;
}

ATerrainTile* ATerrainManager::GetTileAtLocation(FVector WorldLocation) const
{
    FIntPoint TileCoord = WorldToTileCoords(WorldLocation);
    return GetTile(TileCoord.X, TileCoord.Y);
}

ATerrainTile* ATerrainManager::GetTile(int32 TileX, int32 TileY) const
{
    FIntPoint Coord(TileX, TileY);
    return TerrainTiles.FindRef(Coord);
}

FBox ATerrainManager::GetTerrainBounds() const
{
    FVector Min(0.0f, 0.0f, TerrainConfig.MinHeight);
    FVector Max(TerrainConfig.TotalSizeX, TerrainConfig.TotalSizeY, TerrainConfig.MaxHeight);
    return FBox(Min, Max);
}

ATerrainTile* ATerrainManager::CreateTile(int32 TileX, int32 TileY)
{
    if (!AreTileCoordsValid(TileX, TileY))
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ATerrainTile* NewTile = GetWorld()->SpawnActor<ATerrainTile>(ATerrainTile::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (NewTile)
    {
        NewTile->InitializeTile(TileX, TileY, TerrainConfig);
        
#if WITH_EDITOR
        NewTile->SetFolderPath(TEXT("Terrain/Tiles"));
#endif
    }

    return NewTile;
}

FIntPoint ATerrainManager::WorldToTileCoords(FVector WorldLocation) const
{
    int32 TileX = FMath::FloorToInt(WorldLocation.X / TerrainConfig.TileSizeX);
    int32 TileY = FMath::FloorToInt(WorldLocation.Y / TerrainConfig.TileSizeY);
    return FIntPoint(TileX, TileY);
}

bool ATerrainManager::AreTileCoordsValid(int32 TileX, int32 TileY) const
{
    return TileX >= 0 && TileX < TerrainConfig.GetNumTilesX() &&
           TileY >= 0 && TileY < TerrainConfig.GetNumTilesY();
}

TArray<FIntPoint> ATerrainManager::GetTilesInRadius(FVector Center, float Radius) const
{
    TArray<FIntPoint> Result;

    // Convert center and radius to tile coordinates
    FIntPoint CenterTile = WorldToTileCoords(Center);
    int32 TileRadius = FMath::CeilToInt(Radius / FMath::Min(TerrainConfig.TileSizeX, TerrainConfig.TileSizeY));

    for (int32 Y = CenterTile.Y - TileRadius; Y <= CenterTile.Y + TileRadius; Y++)
    {
        for (int32 X = CenterTile.X - TileRadius; X <= CenterTile.X + TileRadius; X++)
        {
            if (AreTileCoordsValid(X, Y))
            {
                // Check actual distance
                float TileCenterX = (X + 0.5f) * TerrainConfig.TileSizeX;
                float TileCenterY = (Y + 0.5f) * TerrainConfig.TileSizeY;
                float Distance = FVector::Dist(Center, FVector(TileCenterX, TileCenterY, Center.Z));
                
                if (Distance <= Radius)
                {
                    Result.Add(FIntPoint(X, Y));
                }
            }
        }
    }

    return Result;
}

void ATerrainManager::ProcessAsyncGeneration(float DeltaTime)
{
    if (!bIsGenerating || TilesToGenerate.Num() == 0)
    {
        return;
    }

    GenerationTimeAccumulator += DeltaTime;

    // Generate tiles while we have time budget
    float StartTime = FPlatformTime::Seconds();
    while (CurrentGenerationIndex < TilesToGenerate.Num())
    {
        const FIntPoint& TileCoord = TilesToGenerate[CurrentGenerationIndex];

        if (!TerrainTiles.Contains(TileCoord))
        {
            ATerrainTile* NewTile = CreateTile(TileCoord.X, TileCoord.Y);
            if (NewTile)
            {
                TerrainTiles.Add(TileCoord, NewTile);
                LoadedTiles.Add(TileCoord);
            }
        }

        CurrentGenerationIndex++;

        // Broadcast progress
        OnTerrainGenerationProgress.Broadcast(CurrentGenerationIndex, TilesToGenerate.Num());

        // Check time budget
        float ElapsedTime = FPlatformTime::Seconds() - StartTime;
        if (ElapsedTime >= MaxGenerationTimePerFrame)
        {
            break;
        }
    }

    // Check if generation is complete
    if (CurrentGenerationIndex >= TilesToGenerate.Num())
    {
        bIsGenerating = false;
        TilesToGenerate.Empty();
        CurrentGenerationIndex = 0;

        UE_LOG(LogTemp, Log, TEXT("TerrainManager: Terrain generation complete! %d tiles created."), TerrainTiles.Num());
        OnTerrainGenerationComplete.Broadcast();
    }
}
