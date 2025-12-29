// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainTile.h"

ATerrainTile::ATerrainTile()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create procedural mesh component
    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
    RootComponent = ProceduralMesh;

    // Enable collision
    ProceduralMesh->bUseComplexAsSimpleCollision = true;

    // Initialize defaults
    TileX = 0;
    TileY = 0;
    CurrentLOD = 0;
    bIsInitialized = false;
}

void ATerrainTile::BeginPlay()
{
    Super::BeginPlay();
}

void ATerrainTile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATerrainTile::InitializeTile(int32 InTileX, int32 InTileY, const FTerrainConfig& InConfig)
{
    TileX = InTileX;
    TileY = InTileY;
    Config = InConfig;

    // Calculate tile world position
    float WorldX = TileX * Config.TileSizeX;
    float WorldY = TileY * Config.TileSizeY;
    SetActorLocation(FVector(WorldX, WorldY, 0.0f));

    // Pre-compute height data for this tile
    int32 Resolution = Config.TileResolution;
    HeightData.SetNum((Resolution + 1) * (Resolution + 1));

    float StepX = Config.TileSizeX / Resolution;
    float StepY = Config.TileSizeY / Resolution;

    for (int32 Y = 0; Y <= Resolution; Y++)
    {
        for (int32 X = 0; X <= Resolution; X++)
        {
            float LocalX = X * StepX;
            float LocalY = Y * StepY;
            float Height = GenerateHeight(WorldX + LocalX, WorldY + LocalY);
            HeightData[Y * (Resolution + 1) + X] = Height;
        }
    }

    bIsInitialized = true;

    // Generate initial mesh at highest LOD
    GenerateMesh();
}

void ATerrainTile::GenerateMesh()
{
    if (!bIsInitialized)
    {
        return;
    }

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;

    GenerateMeshData(CurrentLOD, Vertices, Triangles, Normals, UVs, VertexColors);

    // Clear existing mesh
    ProceduralMesh->ClearAllMeshSections();

    // Create new mesh section
    ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

    // Apply material if set
    if (Config.TerrainMaterial)
    {
        ProceduralMesh->SetMaterial(0, Config.TerrainMaterial);
    }
}

void ATerrainTile::UpdateLOD(FVector CameraLocation)
{
    if (!bIsInitialized)
    {
        return;
    }

    FVector TileCenter = GetTileCenter();
    float Distance = FVector::Dist(CameraLocation, TileCenter);

    int32 NewLOD;
    if (Distance <= Config.LOD0Distance)
    {
        NewLOD = 0;
    }
    else if (Distance <= Config.LOD1Distance)
    {
        NewLOD = 1;
    }
    else if (Distance <= Config.LOD2Distance)
    {
        NewLOD = 2;
    }
    else
    {
        NewLOD = 3;
    }

    if (NewLOD != CurrentLOD)
    {
        CurrentLOD = NewLOD;
        GenerateMesh();
    }
}

FVector ATerrainTile::GetTileCenter() const
{
    FVector Location = GetActorLocation();
    float CenterX = Location.X + Config.TileSizeX * 0.5f;
    float CenterY = Location.Y + Config.TileSizeY * 0.5f;
    float CenterZ = (Config.MaxHeight + Config.MinHeight) * 0.5f;
    return FVector(CenterX, CenterY, CenterZ);
}

float ATerrainTile::GetHeightAtLocation(FVector WorldLocation) const
{
    if (!bIsInitialized || HeightData.Num() == 0)
    {
        return 0.0f;
    }

    FVector TileLocation = GetActorLocation();
    float LocalX = WorldLocation.X - TileLocation.X;
    float LocalY = WorldLocation.Y - TileLocation.Y;

    // Clamp to tile bounds
    LocalX = FMath::Clamp(LocalX, 0.0f, Config.TileSizeX);
    LocalY = FMath::Clamp(LocalY, 0.0f, Config.TileSizeY);

    // Convert to grid coordinates
    int32 Resolution = Config.TileResolution;
    float StepX = Config.TileSizeX / Resolution;
    float StepY = Config.TileSizeY / Resolution;

    float GridX = LocalX / StepX;
    float GridY = LocalY / StepY;

    int32 X0 = FMath::FloorToInt(GridX);
    int32 Y0 = FMath::FloorToInt(GridY);
    int32 X1 = FMath::Min(X0 + 1, Resolution);
    int32 Y1 = FMath::Min(Y0 + 1, Resolution);

    float FracX = GridX - X0;
    float FracY = GridY - Y0;

    // Bilinear interpolation
    float H00 = HeightData[Y0 * (Resolution + 1) + X0];
    float H10 = HeightData[Y0 * (Resolution + 1) + X1];
    float H01 = HeightData[Y1 * (Resolution + 1) + X0];
    float H11 = HeightData[Y1 * (Resolution + 1) + X1];

    float H0 = FMath::Lerp(H00, H10, FracX);
    float H1 = FMath::Lerp(H01, H11, FracX);

    return FMath::Lerp(H0, H1, FracY);
}

bool ATerrainTile::ContainsWorldLocation(FVector WorldLocation) const
{
    FVector TileLocation = GetActorLocation();
    return WorldLocation.X >= TileLocation.X &&
           WorldLocation.X < TileLocation.X + Config.TileSizeX &&
           WorldLocation.Y >= TileLocation.Y &&
           WorldLocation.Y < TileLocation.Y + Config.TileSizeY;
}

void ATerrainTile::SetTerrainMaterial(UMaterialInterface* Material)
{
    Config.TerrainMaterial = Material;
    if (ProceduralMesh)
    {
        ProceduralMesh->SetMaterial(0, Material);
    }
}

float ATerrainTile::GenerateHeight(float WorldX, float WorldY) const
{
    float Height = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Config.NoiseScale;
    float MaxAmplitude = 0.0f;

    for (int32 i = 0; i < Config.NoiseOctaves; i++)
    {
        float NoiseValue = PerlinNoise2D(WorldX * Frequency, WorldY * Frequency, Config.NoiseSeed + i);
        Height += NoiseValue * Amplitude;
        MaxAmplitude += Amplitude;
        Amplitude *= Config.NoisePersistence;
        Frequency *= Config.NoiseLacunarity;
    }

    // Normalize to 0-1 range, then scale to height range
    Height = (Height / MaxAmplitude + 1.0f) * 0.5f;
    Height = FMath::Lerp(Config.MinHeight, Config.MaxHeight, Height);

    return Height;
}

float ATerrainTile::PerlinNoise2D(float X, float Y, int32 Seed) const
{
    // Use FMath::PerlinNoise2D with seed offset
    return FMath::PerlinNoise2D(FVector2D(X + Seed * 0.1f, Y + Seed * 0.1f));
}

void ATerrainTile::GenerateMeshData(int32 LODLevel, TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                     TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FColor>& VertexColors)
{
    // Calculate resolution based on LOD (halve resolution per LOD level)
    int32 LODDivisor = 1 << LODLevel;
    int32 Resolution = FMath::Max(2, Config.TileResolution / LODDivisor);

    float StepX = Config.TileSizeX / Resolution;
    float StepY = Config.TileSizeY / Resolution;

    // Generate vertices
    for (int32 Y = 0; Y <= Resolution; Y++)
    {
        for (int32 X = 0; X <= Resolution; X++)
        {
            float LocalX = X * StepX;
            float LocalY = Y * StepY;

            // Get height from cached data with interpolation
            FVector TileLocation = GetActorLocation();
            float Height = GetHeightAtLocation(FVector(TileLocation.X + LocalX, TileLocation.Y + LocalY, 0.0f));

            Vertices.Add(FVector(LocalX, LocalY, Height));

            // UV coordinates
            UVs.Add(FVector2D((float)X / Resolution, (float)Y / Resolution));

            // Vertex color based on height (can be used for texture blending)
            float HeightNormalized = (Height - Config.MinHeight) / (Config.MaxHeight - Config.MinHeight);
            uint8 HeightByte = FMath::Clamp((int32)(HeightNormalized * 255), 0, 255);
            VertexColors.Add(FColor(HeightByte, HeightByte, HeightByte, 255));
        }
    }

    // Generate triangles
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            int32 TopLeft = Y * (Resolution + 1) + X;
            int32 TopRight = TopLeft + 1;
            int32 BottomLeft = (Y + 1) * (Resolution + 1) + X;
            int32 BottomRight = BottomLeft + 1;

            // First triangle
            Triangles.Add(TopLeft);
            Triangles.Add(BottomLeft);
            Triangles.Add(TopRight);

            // Second triangle
            Triangles.Add(TopRight);
            Triangles.Add(BottomLeft);
            Triangles.Add(BottomRight);
        }
    }

    // Calculate normals
    Normals.SetNum(Vertices.Num());
    for (int32 i = 0; i < Normals.Num(); i++)
    {
        Normals[i] = FVector::ZeroVector;
    }

    // Accumulate face normals for each vertex
    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        int32 i0 = Triangles[i];
        int32 i1 = Triangles[i + 1];
        int32 i2 = Triangles[i + 2];

        FVector V0 = Vertices[i0];
        FVector V1 = Vertices[i1];
        FVector V2 = Vertices[i2];

        FVector Edge1 = V1 - V0;
        FVector Edge2 = V2 - V0;
        FVector FaceNormal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

        Normals[i0] += FaceNormal;
        Normals[i1] += FaceNormal;
        Normals[i2] += FaceNormal;
    }

    // Normalize
    for (int32 i = 0; i < Normals.Num(); i++)
    {
        Normals[i] = Normals[i].GetSafeNormal();
    }
}

FVector ATerrainTile::CalculateNormal(int32 X, int32 Y, int32 Resolution) const
{
    // Get heights of neighboring vertices for normal calculation
    int32 XL = FMath::Max(0, X - 1);
    int32 XR = FMath::Min(Resolution, X + 1);
    int32 YD = FMath::Max(0, Y - 1);
    int32 YU = FMath::Min(Resolution, Y + 1);

    float HL = HeightData[Y * (Resolution + 1) + XL];
    float HR = HeightData[Y * (Resolution + 1) + XR];
    float HD = HeightData[YD * (Resolution + 1) + X];
    float HU = HeightData[YU * (Resolution + 1) + X];

    float StepX = Config.TileSizeX / Resolution;
    float StepY = Config.TileSizeY / Resolution;

    // Gradient
    float dX = (HR - HL) / (2.0f * StepX);
    float dY = (HU - HD) / (2.0f * StepY);

    return FVector(-dX, -dY, 1.0f).GetSafeNormal();
}
