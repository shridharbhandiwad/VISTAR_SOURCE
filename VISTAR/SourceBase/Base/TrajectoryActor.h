// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseActor.h"
#include "TrajectoryActor.generated.h"

class USplineComponent;

UCLASS()
class VISTAR_API ATrajectoryActor : public ABaseActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrajectoryActor();


private :

	UPROPERTY()
	TArray<UChildActorComponent*> listSplineGizmo;

	void addGizmoAtSplinePoint(int nIndex, FVector WorldLoc);

	void ShowHideGizmo(UChildActorComponent* childActorComp);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
	FRotator SplineMeshRotate = FRotator(0, 0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
	FVector SplineMeshScale3D = FVector(1.f, 1.f, 1.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	bool bRouteEditable = false;

	// Skeletal Mesh Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	USplineComponent* SplineComponent;

	// The mesh to use along the spline
	UPROPERTY(EditAnywhere, Category = "Spline")
	UStaticMesh* SplineMesh;  // Assign this in the Blueprint!

	// Mesh material (optional)
	UPROPERTY(EditAnywhere, Category = "Spline")
	UMaterialInterface* SplineMaterial; // Optional material

	void TransmitSelfInfo() override;

	UFUNCTION(BlueprintCallable, Category = "Spline")
	void BuildSplineMeshes();

	UFUNCTION(BlueprintCallable)
	void HandleGizmoUpdate(UChildActorComponent* childActorComp);

	UFUNCTION(BlueprintCallable, Category = "Spline")
	// Add a point in world space
	void AddSplinePointAtLocation(FVector WorldLocation);


	UFUNCTION(BlueprintCallable, Category = "Spline")
	void UpdateSplineMeshes();

	UFUNCTION(BlueprintCallable, Category = "Spline")
	void SetRouteEditable(bool bSplineEditable);

	UFUNCTION(BlueprintCallable, Category = "Spline")
	void StopEditing();

};
