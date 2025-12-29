// Fill out your copyright notice in the Description page of Project Settings.

#include "FoliageManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

AFoliageManager::AFoliageManager()
{
    PrimaryActorTick.bCanEverTick = true;

    bIsInitialized = false;
    bIsGenerating = false;
    CurrentGenerationTileIndex = 0;
    GenerationTimeAccumulator = 0.0f;
    MaxGenerationTimePerFrame = 0.016f; // ~16ms per frame

    // Set up default foliage types
    FFoliageType TreeType;
    TreeType.TypeName = "Tree";
    TreeType.DensityPerKm2 = 500.0f;
    TreeType.MinScale = 0.8f;
    TreeType.MaxScale = 1.5f;
    TreeType.MinSpawnHeight = 0.0f;
    TreeType.MaxSpawnHeight = 40000.0f;
    TreeType.CullDistance = 1000000.0f;
    FoliageTypes.Add(TreeType);

    FFoliageType BushType;
    BushType.TypeName = "Bush";
    BushType.DensityPerKm2 = 1000.0f;
    BushType.MinScale = 0.6f;
    BushType.MaxScale = 1.2f;
    BushType.MinSpawnHeight = 0.0f;
    BushType.MaxSpawnHeight = 35000.0f;
    BushType.CullDistance = 500000.0f;
    FoliageTypes.Add(BushType);
}

void AFoliageManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("FoliageManager: BeginPlay"));
}

void AFoliageManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearAllFoliage();
    Super::EndPlay(EndPlayReason);
}

void AFoliageManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Process async generation
    if (bIsGenerating)
    {
        ProcessAsyncGeneration(DeltaTime);
    }

    // Update streaming
    if (bIsInitialized)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->PlayerCameraManager)
        {
            FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
            UpdateStreaming(CameraLocation);
        }
    }
}

void AFoliageManager::InitializeFoliage()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("FoliageManager: Already initialized"));
        return;
    }

    // Initialize random stream with seed
    RandomStream.Initialize(FoliageConfig.FoliageSeed);

    // Create HISM components for each foliage type that has a mesh
    for (const FFoliageType& FoliageType : FoliageTypes)
    {
        if (FoliageType.Mesh)
        {
            UHierarchicalInstancedStaticMeshComponent* HISM = CreateHISMComponent(FoliageType);
            if (HISM)
            {
                HISMComponents.Add(FoliageType.TypeName, HISM);
                UE_LOG(LogTemp, Log, TEXT("FoliageManager: Created HISM for %s"), *FoliageType.TypeName);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FoliageManager: No mesh set for foliage type %s"), *FoliageType.TypeName);
        }
    }

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Initialized with %d foliage types"), HISMComponents.Num());
}

void AFoliageManager::GenerateAllFoliage()
{
    if (!bIsInitialized)
    {
        InitializeFoliage();
    }

    if (!TerrainManager)
    {
        UE_LOG(LogTemp, Error, TEXT("FoliageManager: No TerrainManager reference set!"));
        return;
    }

    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("FoliageManager: Already generating foliage"));
        return;
    }

    // Queue all tiles for processing
    TilesToProcess.Empty();
    int32 NumTilesX = TerrainManager->TerrainConfig.GetNumTilesX();
    int32 NumTilesY = TerrainManager->TerrainConfig.GetNumTilesY();

    for (int32 Y = 0; Y < NumTilesY; Y++)
    {
        for (int32 X = 0; X < NumTilesX; X++)
        {
            FIntPoint TileCoord(X, Y);
            if (!GeneratedTiles.Contains(TileCoord))
            {
                TilesToProcess.Add(TileCoord);
            }
        }
    }

    CurrentGenerationTileIndex = 0;
    bIsGenerating = true;

    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Starting foliage generation for %d tiles"), TilesToProcess.Num());
}

void AFoliageManager::GenerateFoliageInArea(FVector Center, float Radius)
{
    if (!bIsInitialized)
    {
        InitializeFoliage();
    }

    if (!TerrainManager)
    {
        UE_LOG(LogTemp, Error, TEXT("FoliageManager: No TerrainManager reference set!"));
        return;
    }

    // Find tiles that intersect the area
    float TileSizeX = TerrainManager->TerrainConfig.TileSizeX;
    float TileSizeY = TerrainManager->TerrainConfig.TileSizeY;

    int32 MinTileX = FMath::Max(0, FMath::FloorToInt((Center.X - Radius) / TileSizeX));
    int32 MaxTileX = FMath::Min(TerrainManager->TerrainConfig.GetNumTilesX() - 1, FMath::CeilToInt((Center.X + Radius) / TileSizeX));
    int32 MinTileY = FMath::Max(0, FMath::FloorToInt((Center.Y - Radius) / TileSizeY));
    int32 MaxTileY = FMath::Min(TerrainManager->TerrainConfig.GetNumTilesY() - 1, FMath::CeilToInt((Center.Y + Radius) / TileSizeY));

    int32 GeneratedCount = 0;
    for (int32 Y = MinTileY; Y <= MaxTileY; Y++)
    {
        for (int32 X = MinTileX; X <= MaxTileX; X++)
        {
            FIntPoint TileCoord(X, Y);
            if (!GeneratedTiles.Contains(TileCoord))
            {
                SpawnFoliageForTile(X, Y);
                GeneratedTiles.Add(TileCoord);
                GeneratedCount++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Generated foliage for %d tiles in area"), GeneratedCount);
}

void AFoliageManager::GenerateFoliageForTile(int32 TileX, int32 TileY)
{
    if (!bIsInitialized)
    {
        InitializeFoliage();
    }

    FIntPoint TileCoord(TileX, TileY);
    if (!GeneratedTiles.Contains(TileCoord))
    {
        SpawnFoliageForTile(TileX, TileY);
        GeneratedTiles.Add(TileCoord);
    }
}

void AFoliageManager::ClearAllFoliage()
{
    for (auto& Pair : HISMComponents)
    {
        if (Pair.Value)
        {
            Pair.Value->ClearInstances();
        }
    }
    GeneratedTiles.Empty();

    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Cleared all foliage"));
}

void AFoliageManager::ClearFoliageInArea(FVector Center, float Radius)
{
    // Note: HISM doesn't support efficient area-based removal
    // This is a simplified implementation that clears instances in the area
    // For production use, consider spatial partitioning

    for (auto& Pair : HISMComponents)
    {
        UHierarchicalInstancedStaticMeshComponent* HISM = Pair.Value;
        if (!HISM) continue;

        TArray<int32> InstancesToRemove;
        for (int32 i = 0; i < HISM->GetInstanceCount(); i++)
        {
            FTransform InstanceTransform;
            if (HISM->GetInstanceTransform(i, InstanceTransform, true))
            {
                float Distance = FVector::Dist(InstanceTransform.GetLocation(), Center);
                if (Distance <= Radius)
                {
                    InstancesToRemove.Add(i);
                }
            }
        }

        // Remove in reverse order to preserve indices
        for (int32 i = InstancesToRemove.Num() - 1; i >= 0; i--)
        {
            HISM->RemoveInstance(InstancesToRemove[i]);
        }
    }
}

int32 AFoliageManager::GetTotalInstanceCount() const
{
    int32 Total = 0;
    for (const auto& Pair : HISMComponents)
    {
        if (Pair.Value)
        {
            Total += Pair.Value->GetInstanceCount();
        }
    }
    return Total;
}

void AFoliageManager::UpdateStreaming(FVector CameraPosition)
{
    // HISM handles LOD and culling automatically based on settings
    // This function can be expanded for custom streaming behavior
}

UHierarchicalInstancedStaticMeshComponent* AFoliageManager::CreateHISMComponent(const FFoliageType& FoliageType)
{
    UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, *FoliageType.TypeName);
    if (HISM)
    {
        HISM->SetStaticMesh(FoliageType.Mesh);
        HISM->SetMobility(EComponentMobility::Static);
        HISM->SetCullDistances(0, FoliageType.CullDistance);
        HISM->bAffectDistanceFieldLighting = false;
        HISM->bCastDynamicShadow = false;
        HISM->bCastStaticShadow = true;
        HISM->SetGenerateOverlapEvents(false);
        HISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // Register component
        HISM->RegisterComponent();
        HISM->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    }
    return HISM;
}

void AFoliageManager::SpawnFoliageForTile(int32 TileX, int32 TileY)
{
    if (!TerrainManager)
    {
        return;
    }

    float TileSizeX = TerrainManager->TerrainConfig.TileSizeX;
    float TileSizeY = TerrainManager->TerrainConfig.TileSizeY;
    float TileWorldX = TileX * TileSizeX;
    float TileWorldY = TileY * TileSizeY;

    // Calculate tile seed for deterministic generation
    int32 TileSeed = FoliageConfig.FoliageSeed + TileX * 10000 + TileY;
    FRandomStream TileRandom(TileSeed);

    // Calculate area in km^2
    float TileAreaKm2 = (TileSizeX / 100000.0f) * (TileSizeY / 100000.0f);

    // Spawn each foliage type
    for (int32 TypeIndex = 0; TypeIndex < FoliageTypes.Num(); TypeIndex++)
    {
        const FFoliageType& FoliageType = FoliageTypes[TypeIndex];
        UHierarchicalInstancedStaticMeshComponent** HISMPtr = HISMComponents.Find(FoliageType.TypeName);
        if (!HISMPtr || !*HISMPtr)
        {
            continue;
        }

        UHierarchicalInstancedStaticMeshComponent* HISM = *HISMPtr;

        // Calculate number of instances for this tile
        int32 NumInstances = FMath::RoundToInt(FoliageType.DensityPerKm2 * TileAreaKm2);

        // Calculate minimum distance between instances (for Poisson-like distribution)
        float MinDistance = FMath::Sqrt(TileSizeX * TileSizeY / NumInstances) * 0.5f;

        // Generate spawn positions using Poisson disk sampling
        TArray<FVector2D> SpawnPositions = PoissonDiskSampling(TileSizeX, TileSizeY, MinDistance, TileSeed + TypeIndex, 30);

        // Spawn instances
        TArray<FTransform> Transforms;
        Transforms.Reserve(SpawnPositions.Num());

        for (const FVector2D& LocalPos : SpawnPositions)
        {
            float WorldX = TileWorldX + LocalPos.X;
            float WorldY = TileWorldY + LocalPos.Y;

            // Get height from terrain
            FVector WorldPos(WorldX, WorldY, 0.0f);
            float Height = TerrainManager->GetHeightAtLocation(WorldPos);
            WorldPos.Z = Height;

            // Check height constraints
            if (Height < FoliageType.MinSpawnHeight || Height > FoliageType.MaxSpawnHeight)
            {
                continue;
            }

            // Get surface normal and check slope
            FVector Normal = GetSurfaceNormal(WorldPos);
            float Slope = FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector));
            if (Slope > FoliageType.MaxSpawnSlope)
            {
                continue;
            }

            // Calculate transform
            FTransform InstanceTransform;
            InstanceTransform.SetLocation(WorldPos);

            // Random scale
            float Scale = TileRandom.FRandRange(FoliageType.MinScale, FoliageType.MaxScale);
            InstanceTransform.SetScale3D(FVector(Scale));

            // Random rotation
            FRotator Rotation = FRotator::ZeroRotator;
            if (FoliageType.bRandomYaw)
            {
                Rotation.Yaw = TileRandom.FRandRange(0.0f, 360.0f);
            }
            if (FoliageType.RandomPitchRange > 0.0f)
            {
                Rotation.Pitch = TileRandom.FRandRange(-FoliageType.RandomPitchRange, FoliageType.RandomPitchRange);
                Rotation.Roll = TileRandom.FRandRange(-FoliageType.RandomPitchRange, FoliageType.RandomPitchRange);
            }

            // Align to surface if enabled
            if (FoliageType.bAlignToSurface)
            {
                FVector Forward = FVector::CrossProduct(FVector::RightVector, Normal).GetSafeNormal();
                FVector Right = FVector::CrossProduct(Normal, Forward).GetSafeNormal();
                FMatrix AlignMatrix = FMatrix(Forward, Right, Normal, FVector::ZeroVector);
                FRotator AlignRotation = AlignMatrix.Rotator();
                Rotation = FRotator(FQuat(AlignRotation) * FQuat(Rotation));
            }

            InstanceTransform.SetRotation(FQuat(Rotation));
            Transforms.Add(InstanceTransform);
        }

        // Add all instances at once (more efficient than one at a time)
        if (Transforms.Num() > 0)
        {
            HISM->AddInstances(Transforms, false);
        }
    }
}

TArray<FVector2D> AFoliageManager::GenerateSpawnPositions(float AreaSizeX, float AreaSizeY, float MinDistance, int32 Seed)
{
    return PoissonDiskSampling(AreaSizeX, AreaSizeY, MinDistance, Seed);
}

TArray<FVector2D> AFoliageManager::PoissonDiskSampling(float Width, float Height, float MinDistance, int32 Seed, int32 MaxAttempts)
{
    TArray<FVector2D> Points;
    TArray<FVector2D> ActiveList;

    FRandomStream Random(Seed);

    // Start with a random point
    FVector2D FirstPoint(Random.FRandRange(0.0f, Width), Random.FRandRange(0.0f, Height));
    Points.Add(FirstPoint);
    ActiveList.Add(FirstPoint);

    // Cell size for spatial hashing
    float CellSize = MinDistance / FMath::Sqrt(2.0f);
    int32 GridWidth = FMath::CeilToInt(Width / CellSize);
    int32 GridHeight = FMath::CeilToInt(Height / CellSize);

    // Grid for spatial lookup (-1 means empty)
    TArray<int32> Grid;
    Grid.Init(-1, GridWidth * GridHeight);

    // Add first point to grid
    int32 GridX = FMath::FloorToInt(FirstPoint.X / CellSize);
    int32 GridY = FMath::FloorToInt(FirstPoint.Y / CellSize);
    Grid[GridY * GridWidth + GridX] = 0;

    while (ActiveList.Num() > 0)
    {
        // Pick random point from active list
        int32 ActiveIndex = Random.RandRange(0, ActiveList.Num() - 1);
        FVector2D Point = ActiveList[ActiveIndex];

        bool bFoundValid = false;

        for (int32 i = 0; i < MaxAttempts; i++)
        {
            // Generate random point in annulus around Point
            float Angle = Random.FRandRange(0.0f, 2.0f * PI);
            float Distance = Random.FRandRange(MinDistance, 2.0f * MinDistance);
            FVector2D NewPoint = Point + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Distance;

            // Check bounds
            if (NewPoint.X < 0 || NewPoint.X >= Width || NewPoint.Y < 0 || NewPoint.Y >= Height)
            {
                continue;
            }

            // Check distance to nearby points using grid
            int32 NewGridX = FMath::FloorToInt(NewPoint.X / CellSize);
            int32 NewGridY = FMath::FloorToInt(NewPoint.Y / CellSize);

            bool bValid = true;
            for (int32 dy = -2; dy <= 2 && bValid; dy++)
            {
                for (int32 dx = -2; dx <= 2 && bValid; dx++)
                {
                    int32 CheckX = NewGridX + dx;
                    int32 CheckY = NewGridY + dy;

                    if (CheckX >= 0 && CheckX < GridWidth && CheckY >= 0 && CheckY < GridHeight)
                    {
                        int32 PointIndex = Grid[CheckY * GridWidth + CheckX];
                        if (PointIndex != -1)
                        {
                            float Dist = FVector2D::Distance(NewPoint, Points[PointIndex]);
                            if (Dist < MinDistance)
                            {
                                bValid = false;
                            }
                        }
                    }
                }
            }

            if (bValid)
            {
                Points.Add(NewPoint);
                ActiveList.Add(NewPoint);
                Grid[NewGridY * GridWidth + NewGridX] = Points.Num() - 1;
                bFoundValid = true;
                break;
            }
        }

        if (!bFoundValid)
        {
            // Remove point from active list
            ActiveList.RemoveAt(ActiveIndex);
        }
    }

    return Points;
}

FVector AFoliageManager::GetSurfaceNormal(FVector Position) const
{
    if (!TerrainManager)
    {
        return FVector::UpVector;
    }

    // Sample heights at nearby points to calculate normal
    float SampleDist = 100.0f; // 1 meter

    float HeightC = TerrainManager->GetHeightAtLocation(Position);
    float HeightL = TerrainManager->GetHeightAtLocation(Position + FVector(-SampleDist, 0, 0));
    float HeightR = TerrainManager->GetHeightAtLocation(Position + FVector(SampleDist, 0, 0));
    float HeightD = TerrainManager->GetHeightAtLocation(Position + FVector(0, -SampleDist, 0));
    float HeightU = TerrainManager->GetHeightAtLocation(Position + FVector(0, SampleDist, 0));

    FVector TangentX(2.0f * SampleDist, 0, HeightR - HeightL);
    FVector TangentY(0, 2.0f * SampleDist, HeightU - HeightD);

    return FVector::CrossProduct(TangentY, TangentX).GetSafeNormal();
}

void AFoliageManager::ProcessAsyncGeneration(float DeltaTime)
{
    if (!bIsGenerating || TilesToProcess.Num() == 0)
    {
        return;
    }

    float StartTime = FPlatformTime::Seconds();

    while (CurrentGenerationTileIndex < TilesToProcess.Num())
    {
        const FIntPoint& TileCoord = TilesToProcess[CurrentGenerationTileIndex];
        SpawnFoliageForTile(TileCoord.X, TileCoord.Y);
        GeneratedTiles.Add(TileCoord);
        CurrentGenerationTileIndex++;

        // Broadcast progress
        OnFoliageGenerationProgress.Broadcast(CurrentGenerationTileIndex, TilesToProcess.Num());

        // Check time budget
        float ElapsedTime = FPlatformTime::Seconds() - StartTime;
        if (ElapsedTime >= MaxGenerationTimePerFrame)
        {
            break;
        }
    }

    // Check if complete
    if (CurrentGenerationTileIndex >= TilesToProcess.Num())
    {
        bIsGenerating = false;
        TilesToProcess.Empty();
        CurrentGenerationTileIndex = 0;

        int32 TotalInstances = GetTotalInstanceCount();
        UE_LOG(LogTemp, Log, TEXT("FoliageManager: Generation complete! Total instances: %d"), TotalInstances);
        OnFoliageGenerationComplete.Broadcast(TotalInstances);
    }
}
