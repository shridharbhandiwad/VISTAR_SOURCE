// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseActor.h"
#include "VistarActor.generated.h"

UCLASS()
class VISTAR_API AVistarActor : public ABaseActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVistarActor();

private :
	FString sAttachedTrajectoryName = "";

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Skeletal Mesh Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* SkeletalMesh;

	UFUNCTION(BlueprintCallable, Category = "Trajectory")
	void AttachTrajectory(FString trajectoryName);


	virtual void TransmitSelfInfo() override;

};
