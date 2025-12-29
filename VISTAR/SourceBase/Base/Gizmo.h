#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gizmo.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifyParentEvent, UChildActorComponent*, ParentChildActorComponent);

UCLASS()
class VISTAR_API AGizmo : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AGizmo();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Arrow_X;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Arrow_Y;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Arrow_Z;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Edge_XY_X;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Edge_XY_Y;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Edge_YZ_Y;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Edge_YZ_Z;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Edge_XZ_X;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Edge_XZ_Z;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gizmo")
	UStaticMeshComponent* Sphere_XYZ;

	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void Show();
	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void Hide();
	UFUNCTION(BlueprintCallable, Category = "Gizmo")
	void Release();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigParams")
	double ScaleFactor = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigParams")
	FName ColorVectorParamName = "Color";


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigParams")
	bool bUpdateParent = true;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetUpdateParentComponent(bool);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNotifyParentEvent onNotifyParent;


private :

	bool _m_bHoverX;
	bool _m_bHoverY;
	bool _m_bHoverZ;

	bool _m_bTranslateX;
	bool _m_bTranslateY;
	bool _m_bTranslateZ;

	FLinearColor _m_LinearColourRed = FLinearColor(1.0f,0.0f,0.0f);
	FLinearColor _m_LinearColourGreen = FLinearColor(0.0f, 1.0f, 0.0f);
	FLinearColor _m_LinearColourBlue = FLinearColor(0.0f, 0.0f, 1.0f);
	FLinearColor _m_LinearColourYellow = FLinearColor(1.0f, 1.0f, 0.0f);
	FLinearColor _m_LinearColourWhite = FLinearColor(1.0f, 1.0f, 1.0f);

	TMap<UMeshComponent*, TArray<UMaterialInstanceDynamic*>> MeshMaterialCache;
	void SetupDynamicMaterials(UMeshComponent* MeshComp);

	bool _m_bLockGizmo;
	FVector _m_GizmoWorldPos;

	void LockGizmoPosition();

	void UpdateMeshMaterials();

	void SetMeshColor(UStaticMeshComponent* MeshComp, FLinearColor NewColor);

	UFUNCTION()
	void OnMeshBeginCursorOver_Arrow_X(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnMeshBeginCursorOver_Arrow_Y(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnMeshBeginCursorOver_Arrow_Z(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnMeshBeginCursorOver_Edges_XY(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnMeshBeginCursorOver_Edges_YZ(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnMeshBeginCursorOver_Edges_XZ(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnMeshBeginCursorOver_Sphere_XYZ(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnMeshEndCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnMeshClicked_Arrow_X(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnMeshClicked_Arrow_Y(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnMeshClicked_Arrow_Z(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnMeshClicked_Edges_XY(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnMeshClicked_Edges_YZ(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnMeshClicked_Edges_XZ(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnMeshClicked_Sphere_XYZ(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	
	UFUNCTION(BlueprintCallable)
	void NotifyParentUpdateEvent();

};
