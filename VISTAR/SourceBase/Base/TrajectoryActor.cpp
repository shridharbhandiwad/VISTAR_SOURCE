// Fill out your copyright notice in the Description page of Project Settings.


#include "TrajectoryActor.h"
#include "Components/SplineComponent.h"
#include "VistarGameInstance.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Gizmo.h"

// Sets default values
ATrajectoryActor::ATrajectoryActor()
{
	// Create spline component and attach to root
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));

	SplineComponent->SetupAttachment(RootComponent);
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//BuildSplineMeshes();

}

// Called when the game starts or when spawned
void ATrajectoryActor::BeginPlay()
{
	Super::BeginPlay();
	TArray<UChildActorComponent*> ExistingMeshes;
	GetComponents(ExistingMeshes);
	for (auto MeshComp : ExistingMeshes)
	{
		//if (AGizmo* gizmo = Cast<AGizmo>(MeshComp->GetChildActor())) {
		listSplineGizmo.Remove(MeshComp);
		MeshComp->DestroyComponent();
		//}
	}
	listSplineGizmo.Empty();

	UpdateSplineMeshes();
	for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints(); i++)
	{
		FVector Loc = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		addGizmoAtSplinePoint(i, Loc);
	}
	
}

// Called every frame
void ATrajectoryActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATrajectoryActor::addGizmoAtSplinePoint(int nIndex, FVector WorldLoc)
{
	UChildActorComponent* childActorComp = NewObject<UChildActorComponent>(this);
	childActorComp->SetupAttachment(RootComponent);
	childActorComp->SetChildActorClass(AGizmo::StaticClass());
	childActorComp->RegisterComponent();
	childActorComp->SetWorldLocation(WorldLoc);

	if (AGizmo* Gizmo = Cast<AGizmo>(childActorComp->GetChildActor()))
	{
		Gizmo->SetUpdateParentComponent(false);
		Gizmo->onNotifyParent.AddDynamic(this, &ATrajectoryActor::HandleGizmoUpdate);
	}
	listSplineGizmo.Add(childActorComp);
	ShowHideGizmo(childActorComp);
}

void ATrajectoryActor::SetRouteEditable(bool bSplineEditable) {

	bRouteEditable = bSplineEditable;
	for (UChildActorComponent* childActorComp : listSplineGizmo) {
		ShowHideGizmo(childActorComp);
	}
}

void ATrajectoryActor::StopEditing() {

	for (UChildActorComponent* childActorComp : listSplineGizmo) {
		AActor* childActor = childActorComp->GetChildActor();
		if (childActor) {
			if (AGizmo* gizmo = Cast<AGizmo>(childActor))
			{
				gizmo->Release();
			}
		}
	}
}

void ATrajectoryActor::HandleGizmoUpdate(UChildActorComponent* childActorComp) {

	int32 Index = listSplineGizmo.Find(childActorComp);
	if (Index != INDEX_NONE) {

		SplineComponent->SetLocationAtSplinePoint(Index, childActorComp->GetComponentLocation(), ESplineCoordinateSpace::World, false);
		UpdateSplineMeshes();
	}
}

void ATrajectoryActor::ShowHideGizmo(UChildActorComponent* childActorComp) {

	if (childActorComp) {
		AActor* childActor = childActorComp->GetChildActor();
		if (childActor) {
			if (AGizmo* gizmo = Cast<AGizmo>(childActor))
			{
				if (bRouteEditable) {
					gizmo->Show();
				}
				else {
					gizmo->Hide();
					gizmo->Release();
				}
			}
		}
	}
}

void ATrajectoryActor::AddSplinePointAtLocation(FVector WorldLocation)
{
	int32 Index = SplineComponent->GetNumberOfSplinePoints();
	SplineComponent->AddSplinePoint(WorldLocation, ESplineCoordinateSpace::World, false);
	UpdateSplineMeshes();
	addGizmoAtSplinePoint(Index, WorldLocation);
}

void ATrajectoryActor::UpdateSplineMeshes()
{
	// Remove existing mesh components
	TArray<USplineMeshComponent*> ExistingMeshes;
	GetComponents(ExistingMeshes);
	for (auto MeshComp : ExistingMeshes)
	{
		MeshComp->DestroyComponent();
	}

	// Create new mesh segments
	for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints() - 1; i++)
	{

		FVector StartLoc, StartTangent, EndLoc, EndTangent;
		StartLoc = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		StartTangent = SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
		EndLoc = SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);
		EndTangent = SplineComponent->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World);

		USplineMeshComponent* SplineMeshComp = NewObject<USplineMeshComponent>(this);
		if (SplineMesh)
		{
			SplineMeshComp->SetStaticMesh(SplineMesh);
		}
		if (SplineMaterial)
		{
			SplineMeshComp->SetMaterial(0, SplineMaterial);
		}
		SplineMeshComp->RegisterComponent();
		SplineMeshComp->SetMobility(EComponentMobility::Movable);
		SplineMeshComp->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepWorldTransform);

		SplineMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SplineMeshComp->SetCollisionObjectType(ECC_WorldStatic);
		SplineMeshComp->SetCollisionResponseToAllChannels(ECR_Block);

		// Optional: Use complex collision from mesh if needed
		SplineMeshComp->SetCollisionProfileName(TEXT("BlockAll"));
		SplineComponent->SetSplinePointType(i, ESplinePointType::Curve, true);
		SplineComponent->SetSplinePointType(i + 1, ESplinePointType::Curve, true);
		SplineMeshComp->SetStartAndEnd(StartLoc, StartTangent, EndLoc, EndTangent, true);

		//SplineComp->SetSplinePointType(i, ESplinePointType::CurveClamped, true); // for ends
	}
	SplineComponent->UpdateSpline();

}
void ATrajectoryActor::BuildSplineMeshes()
{
	// Destroy existing spline mesh components
	TArray<USplineMeshComponent*> OldComponents;
	GetComponents<USplineMeshComponent>(OldComponents);
	for (UActorComponent* Comp : OldComponents)
	{
		Comp->DestroyComponent();
	}

	const int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
	for (int32 i = 0; i < NumPoints - 1; ++i)
	{
		FVector StartPos = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		FVector StartTangent = SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		FVector EndPos = SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);
		FVector EndTangent = SplineComponent->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);

		USplineMeshComponent* SplineMeshComp = NewObject<USplineMeshComponent>(this);
		SplineMeshComp->RegisterComponent();
		SplineMeshComp->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);

		if (SplineMesh)
		{
			SplineMeshComp->SetStaticMesh(SplineMesh);
		}

		if (SplineMaterial)
		{
			SplineMeshComp->SetMaterial(0, SplineMaterial);
		}

		SplineMeshComp->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
	}
}

void ATrajectoryActor::TransmitSelfInfo() {

	FString JsonOutput;

	TSharedRef<FJsonObject> JsonObjectRoot = MakeShared<FJsonObject>();

	int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();

	TArray<TSharedPtr<FJsonValue>> JsonObjectLocationsList;

	for (int32 i = 0; i < NumPoints; ++i)
	{
		FVector location = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

		TSharedRef<FJsonObject> JsonObjectLocation = MakeShared<FJsonObject>();
		JsonObjectLocation->SetNumberField(TEXT("X"), location.X);
		JsonObjectLocation->SetNumberField(TEXT("Y"), location.Y);
		JsonObjectLocation->SetNumberField(TEXT("Z"), location.Z);
		JsonObjectLocationsList.Add(MakeShared<FJsonValueObject>(JsonObjectLocation));
	}


	// Add key-value pairs
	JsonObjectRoot->SetStringField(TEXT("ID"), sObjectId);
	JsonObjectRoot->SetStringField(TEXT("CLASS"), sObjectClass);
	JsonObjectRoot->SetStringField(TEXT("STREAM"), TEXT("Create"));
	JsonObjectRoot->SetArrayField(TEXT("POINTS"), JsonObjectLocationsList);

	// Convert to string
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonOutput);
	FJsonSerializer::Serialize(JsonObjectRoot, Writer);

	UVistarGameInstance* VistarGI = Cast<UVistarGameInstance>(GetGameInstance());

	if (VistarGI)
	{
		VistarGI->SendMessage(JsonOutput);
	}
}