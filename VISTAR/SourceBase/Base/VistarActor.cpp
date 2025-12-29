// Fill out your copyright notice in the Description page of Project Settings.


#include "VistarActor.h"
#include "VistarGameInstance.h"
// Sets default values
AVistarActor::AVistarActor()
{
	// Create the skeletal mesh component
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));

	// Attach skeletal mesh to the root
	SkeletalMesh->SetupAttachment(RootComponent);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AVistarActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVistarActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVistarActor::AttachTrajectory(FString trajectoryName) {
	sAttachedTrajectoryName = trajectoryName;
}

void AVistarActor::TransmitSelfInfo() {
	
	FVector location = GetActorLocation();
	FRotator rotation = GetActorRotation();
	FString JsonOutput;

	TSharedRef<FJsonObject> JsonObjectRoot = MakeShared<FJsonObject>();

	TSharedRef<FJsonObject> JsonObjectLocation = MakeShared<FJsonObject>();
	JsonObjectLocation->SetNumberField("X", location.X);
	JsonObjectLocation->SetNumberField(TEXT("Y"), location.Y);
	JsonObjectLocation->SetNumberField(TEXT("Z"), location.Z);

	TSharedRef<FJsonObject> JsonObjectRotation = MakeShared<FJsonObject>();
	JsonObjectRotation->SetNumberField(TEXT("YAW"), rotation.Yaw);
	JsonObjectRotation->SetNumberField(TEXT("PITCH"), rotation.Pitch);
	JsonObjectRotation->SetNumberField(TEXT("ROLL"), rotation.Roll);


	// Add key-value pairs
	JsonObjectRoot->SetStringField(TEXT("ID"), sObjectId);
	JsonObjectRoot->SetStringField(TEXT("CLASS"), sObjectClass);
	JsonObjectRoot->SetStringField(TEXT("STREAM"), TEXT("Create"));
	JsonObjectRoot->SetStringField(TEXT("TRAJECTORY"), sAttachedTrajectoryName);
	JsonObjectRoot->SetObjectField(TEXT("LOCATION"), JsonObjectLocation);
	JsonObjectRoot->SetObjectField(TEXT("ROTATION"), JsonObjectRotation);

	// Convert to string
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonOutput);
	FJsonSerializer::Serialize(JsonObjectRoot, Writer);

	UVistarGameInstance* VistarGI = Cast<UVistarGameInstance>(GetGameInstance());

	if (VistarGI)
	{
		VistarGI->SendMessage(JsonOutput);
	}
}