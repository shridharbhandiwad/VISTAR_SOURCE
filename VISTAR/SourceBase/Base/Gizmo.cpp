#include "Gizmo.h"
#include "Kismet/KismetMathLibrary.h"
// Sets default values
AGizmo::AGizmo()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_m_bHoverX = false;
	_m_bHoverY = false;
	_m_bHoverZ = false;

	_m_bTranslateX = false;
	_m_bTranslateY = false;
	_m_bTranslateZ = false;

	_m_bLockGizmo = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));


	Edge_XZ_Z = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Edge_XZ_Z"));
	Edge_XZ_Z->SetupAttachment(RootComponent);
	Edge_XZ_Z->SetRelativeLocation(FVector(6.f, 0.f, 13.f));
	Edge_XZ_Z->SetRelativeRotation(FRotator(0.f, 0.f, 0.f)); // Point along +XZ
	Edge_XZ_Z->SetRelativeScale3D(FVector(1.5f, 2.f, 2.f));
	// Bind mouse events
	Edge_XZ_Z->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Edges_XZ);
	Edge_XZ_Z->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Edge_XZ_Z->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Edges_XZ);


	Edge_XZ_X = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Edge_XZ_X"));
	Edge_XZ_X->SetupAttachment(RootComponent);
	Edge_XZ_X->SetRelativeLocation(FVector(13.f, 0.f, 6.f));
	Edge_XZ_X->SetRelativeRotation(FRotator(90.f, 0.f, 0.f)); // Point along +XZ
	Edge_XZ_X->SetRelativeScale3D(FVector(1.5f, 2.f, 2.f));
	// Bind mouse events
	Edge_XZ_X->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Edges_XZ);
	Edge_XZ_X->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Edge_XZ_X->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Edges_XZ);




	Edge_YZ_Z = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Edge_YZ_Z"));
	Edge_YZ_Z->SetupAttachment(RootComponent);
	Edge_YZ_Z->SetRelativeLocation(FVector(0.f, 6.f, 13.f));
	Edge_YZ_Z->SetRelativeRotation(FRotator(0.f, 90.f, 0.f)); // Point along +YZ
	Edge_YZ_Z->SetRelativeScale3D(FVector(1.5f, 2.f, 2.f));
	// Bind mouse events
	Edge_YZ_Z->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Edges_YZ);
	Edge_YZ_Z->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Edge_YZ_Z->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Edges_YZ);


	Edge_YZ_Y = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Edge_YZ_Y"));
	Edge_YZ_Y->SetupAttachment(RootComponent);
	Edge_YZ_Y->SetRelativeLocation(FVector(0.f, 13.f, 6.f));
	Edge_YZ_Y->SetRelativeRotation(FRotator(90.f, 0.f, 0.f)); // Point along +YZ
	Edge_YZ_Y->SetRelativeScale3D(FVector(1.5f, 2.f, 2.f));
	// Bind mouse events
	Edge_YZ_Y->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Edges_YZ);
	Edge_YZ_Y->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Edge_YZ_Y->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Edges_YZ);




	Edge_XY_Y = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Edge_XY_Y"));
	Edge_XY_Y->SetupAttachment(RootComponent);
	Edge_XY_Y->SetRelativeLocation(FVector(6.f, 13.f, 0.f));
	Edge_XY_Y->SetRelativeRotation(FRotator(0.f, 0.f, 0.f)); // Point along +XY
	Edge_XY_Y->SetRelativeScale3D(FVector(1.5f, 2.f, 2.f));
	// Bind mouse events
	Edge_XY_Y->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Edges_XY);
	Edge_XY_Y->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Edge_XY_Y->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Edges_XY);



	Edge_XY_X = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Edge_XY_X"));
	Edge_XY_X->SetupAttachment(RootComponent);
	Edge_XY_X->SetRelativeLocation(FVector(13.f, 6.f, 0.f));
	Edge_XY_X->SetRelativeRotation(FRotator(0.f, 90.f, 0.f)); // Point along +XY
	Edge_XY_X->SetRelativeScale3D(FVector(1.5f, 2.f, 2.f));
	// Bind mouse events
	Edge_XY_X->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Edges_XY);
	Edge_XY_X->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Edge_XY_X->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Edges_XY);




	Arrow_Z = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow_Z"));
	Arrow_Z->SetupAttachment(RootComponent);
	Arrow_Z->SetRelativeLocation(FVector(0.f, 0.f, 4.f));
	Arrow_Z->SetRelativeRotation(FRotator(90.f, 0.f, 00.f)); // Point along +Z
	// Bind mouse events
	Arrow_Z->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Arrow_Z);
	Arrow_Z->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Arrow_Z->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Arrow_Z);


	Arrow_Y = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow_Y"));
	Arrow_Y->SetupAttachment(RootComponent);
	Arrow_Y->SetRelativeLocation(FVector(0.f, 4.f, 0.f));
	Arrow_Y->SetRelativeRotation(FRotator(0.f, 90.f, 0.f)); // Point along +Y
	// Bind mouse events
	Arrow_Y->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Arrow_Y);
	Arrow_Y->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Arrow_Y->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Arrow_Y);


	Arrow_X = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow_X"));
	Arrow_X->SetupAttachment(RootComponent);
	Arrow_X->SetRelativeLocation(FVector(4.f, 0.f, 0.f));
	Arrow_X->SetRelativeRotation(FRotator(0.f, 0.f, 0.f)); // Point along +X
	// Bind mouse events
	Arrow_X->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Arrow_X);
	Arrow_X->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Arrow_X->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Arrow_X);


	Sphere_XYZ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere_XYZ"));
	Sphere_XYZ->SetupAttachment(RootComponent);
	//Sphere_XYZ->SetRelativeLocation(FVector(3.f, 3.f, 3.f));
	Sphere_XYZ->SetRelativeRotation(FRotator(0.f, 0.f, 0.f)); // Point along +XZY
	Sphere_XYZ->SetRelativeScale3D(FVector(6.f, 6.f, 6.f));
	// Bind mouse events
	Sphere_XYZ->OnBeginCursorOver.AddDynamic(this, &AGizmo::OnMeshBeginCursorOver_Sphere_XYZ);
	Sphere_XYZ->OnEndCursorOver.AddDynamic(this, &AGizmo::OnMeshEndCursorOver);
	Sphere_XYZ->OnClicked.AddDynamic(this, &AGizmo::OnMeshClicked_Sphere_XYZ);


	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMeshFinder(TEXT("/Engine/VREditor/TransformGizmo/TranslateArrowHandle"));
	if (ArrowMeshFinder.Succeeded()) {
		Arrow_X->SetStaticMesh(ArrowMeshFinder.Object);
		Arrow_Y->SetStaticMesh(ArrowMeshFinder.Object);
		Arrow_Z->SetStaticMesh(ArrowMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> EdgeMeshFinder(TEXT("/Engine/VREditor/TransformGizmo/BoundingBoxEdge"));
	if (EdgeMeshFinder.Succeeded()) {
		Edge_XY_X->SetStaticMesh(EdgeMeshFinder.Object);
		Edge_XY_Y->SetStaticMesh(EdgeMeshFinder.Object);
		Edge_YZ_Y->SetStaticMesh(EdgeMeshFinder.Object);
		Edge_YZ_Z->SetStaticMesh(EdgeMeshFinder.Object);
		Edge_XZ_X->SetStaticMesh(EdgeMeshFinder.Object);
		Edge_XZ_Z->SetStaticMesh(EdgeMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/VREditor/TransformGizmo/TransformGizmoFreeRotation"));
	if (SphereMeshFinder.Succeeded()) {
		Sphere_XYZ->SetStaticMesh(SphereMeshFinder.Object);
	}

	
}



// Called when the game starts or when spawned
void AGizmo::BeginPlay()
{
	Super::BeginPlay();

	APlayerController *PC = GetWorld()->GetFirstPlayerController();
	if (PC) {
		PC->bEnableMouseOverEvents = true;
	}
	UpdateMeshMaterials();
	Hide();
}

// Called every frame
void AGizmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC) {
		FVector vectorScale(0.0f, 0.0f, 0.0f);
		if (!IsHidden()) {
			FVector WorldPosViewPoint;
			FRotator WorldRotViewPoint;
			PC->GetPlayerViewPoint(WorldPosViewPoint, WorldRotViewPoint);
			double dCamFOV = UKismetMathLibrary::DegreesToRadians(PC->PlayerCameraManager->GetFOVAngle())/2.0;
			double distance = UKismetMathLibrary::Vector_Distance(WorldPosViewPoint, GetActorLocation());

			double dScale = fmax(UKismetMathLibrary::Tan(dCamFOV)* distance * ScaleFactor * 0.002,5);
			vectorScale = FVector(dScale, dScale, dScale);
		}
		SetActorScale3D(vectorScale);

		if (_m_bTranslateX || _m_bTranslateY || _m_bTranslateZ) {

			FVector2D MousePos;
			FVector WorldPos;
			FVector WorldDir;
			PC->GetMousePosition(MousePos.X, MousePos.Y);
			PC->DeprojectScreenPositionToWorld(MousePos.X, MousePos.Y, WorldPos, WorldDir);
			FVector ActorLoc;
			if (bUpdateParent) {
				ActorLoc = GetActorLocation();
			}
			else {
				ActorLoc = GetParentComponent()->GetComponentLocation();
			}
			FVector cameraLoc = PC->PlayerCameraManager->GetCameraLocation();
			double distance = UKismetMathLibrary::Vector_Distance(cameraLoc, ActorLoc);
			double distanceFactor = FMath::Max(distance / 12, 120);

			FVector deltaPosition = (WorldPos - _m_GizmoWorldPos) * FVector(_m_bTranslateX, _m_bTranslateY, _m_bTranslateZ) * FVector(distanceFactor, distanceFactor, distanceFactor);
			_m_GizmoWorldPos = WorldPos;
			FVector NewPosition = ActorLoc + deltaPosition;
			
			if ( bUpdateParent ) {
				GetParentActor()->SetActorLocation(NewPosition);
			}
			else {
				GetParentComponent()->SetWorldLocation(NewPosition);
				NotifyParentUpdateEvent();
			}
		}
	}
}


void AGizmo::SetUpdateParentComponent(bool bUpdateParentComponent) {
	bUpdateParent = bUpdateParentComponent;
}

void AGizmo::OnMeshBeginCursorOver_Arrow_X(UPrimitiveComponent* TouchedComponent) {
	_m_bHoverX = true;
	_m_bHoverY = false;
	_m_bHoverZ = false;
	UpdateMeshMaterials();
}

void AGizmo::OnMeshBeginCursorOver_Arrow_Y(UPrimitiveComponent* TouchedComponent) {
	_m_bHoverX = false;
	_m_bHoverY = true;
	_m_bHoverZ = false;
	UpdateMeshMaterials();
}

void AGizmo::OnMeshBeginCursorOver_Arrow_Z(UPrimitiveComponent* TouchedComponent) {
	_m_bHoverX = false;
	_m_bHoverY = false;
	_m_bHoverZ = true;
	UpdateMeshMaterials();
}

void AGizmo::OnMeshBeginCursorOver_Edges_XY(UPrimitiveComponent* TouchedComponent) {
	_m_bHoverX = true;
	_m_bHoverY = true;
	_m_bHoverZ = false;
	UpdateMeshMaterials();
}

void AGizmo::OnMeshBeginCursorOver_Edges_YZ(UPrimitiveComponent* TouchedComponent) {
	_m_bHoverX = false;
	_m_bHoverY = true;
	_m_bHoverZ = true;
	UpdateMeshMaterials();
}

void AGizmo::OnMeshBeginCursorOver_Edges_XZ(UPrimitiveComponent* TouchedComponent) {
	_m_bHoverX = true;
	_m_bHoverY = false;
	_m_bHoverZ = true;
	UpdateMeshMaterials();
}

void AGizmo::OnMeshBeginCursorOver_Sphere_XYZ(UPrimitiveComponent* TouchedComponent) {
	_m_bHoverX = true;
	_m_bHoverY = true;
	_m_bHoverZ = true;
	UpdateMeshMaterials();
}

void AGizmo::OnMeshEndCursorOver(UPrimitiveComponent* TouchedComponent) {
	if ( !_m_bTranslateX ) {
		_m_bHoverX = false;
	}
	if (!_m_bTranslateY) {
		_m_bHoverY = false;
	}
	if (!_m_bTranslateZ) {
		_m_bHoverZ = false;
	}
	UpdateMeshMaterials();
}

void AGizmo::OnMeshClicked_Arrow_X(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed) {
	_m_bTranslateX = true;
	_m_bTranslateY = false;
	_m_bTranslateZ = false;
	LockGizmoPosition();
}

void AGizmo::OnMeshClicked_Arrow_Y(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed) {
	_m_bTranslateX = false;
	_m_bTranslateY = true;
	_m_bTranslateZ = false;
	LockGizmoPosition();
}

void AGizmo::OnMeshClicked_Arrow_Z(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed) {
	_m_bTranslateX = false;
	_m_bTranslateY = false;
	_m_bTranslateZ = true;
	LockGizmoPosition();
}

void AGizmo::OnMeshClicked_Edges_XY(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed) {
	_m_bTranslateX = true;
	_m_bTranslateY = true;
	_m_bTranslateZ = false;
	LockGizmoPosition();
}

void AGizmo::OnMeshClicked_Edges_YZ(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed) {
	_m_bTranslateX = false;
	_m_bTranslateY = true;
	_m_bTranslateZ = true;
	LockGizmoPosition();
}

void AGizmo::OnMeshClicked_Edges_XZ(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed) {
	_m_bTranslateX = true;
	_m_bTranslateY = false;
	_m_bTranslateZ = true;
	LockGizmoPosition();
}

void AGizmo::OnMeshClicked_Sphere_XYZ(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed) {
	_m_bTranslateX = true;
	_m_bTranslateY = true;
	_m_bTranslateZ = true;
	LockGizmoPosition();
}

void AGizmo::Release()
{
	_m_bHoverX = false;
	_m_bHoverY = false;
	_m_bHoverZ = false;
	_m_bTranslateX = false;
	_m_bTranslateY = false;
	_m_bTranslateZ = false;
	_m_bLockGizmo = false;
	UpdateMeshMaterials();
}

void AGizmo::Show()
{
	SetActorTickEnabled(true);
	SetActorHiddenInGame(false);
	//GetParentActor()->SetActorEnableCollision(false);
	SetActorEnableCollision(true);
}

void AGizmo::Hide()
{
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	//GetParentActor()->SetActorEnableCollision(true);
	SetActorEnableCollision(false);
}

void AGizmo::LockGizmoPosition() {
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC) {
		_m_bLockGizmo = true;
		FVector2D MousePos;
		FVector WorldDir;
		PC->GetMousePosition(MousePos.X, MousePos.Y);
		PC->DeprojectScreenPositionToWorld(MousePos.X, MousePos.Y, _m_GizmoWorldPos, WorldDir);
	}
}

void AGizmo::UpdateMeshMaterials() {

	if (!_m_bLockGizmo) {

		SetMeshColor(Sphere_XYZ, (_m_bHoverX && _m_bHoverY && _m_bHoverZ) ? _m_LinearColourYellow : _m_LinearColourWhite);

		SetMeshColor(Arrow_X, _m_bHoverX ? _m_LinearColourYellow : _m_LinearColourRed);
		SetMeshColor(Arrow_Y, _m_bHoverY ? _m_LinearColourYellow : _m_LinearColourGreen);
		SetMeshColor(Arrow_Z, _m_bHoverZ ? _m_LinearColourYellow : _m_LinearColourBlue);

		SetMeshColor(Edge_XY_X, (_m_bHoverX && _m_bHoverY) ? _m_LinearColourYellow : _m_LinearColourRed);
		SetMeshColor(Edge_XY_Y, (_m_bHoverX && _m_bHoverY) ? _m_LinearColourYellow : _m_LinearColourGreen);

		SetMeshColor(Edge_YZ_Y, (_m_bHoverY && _m_bHoverZ) ? _m_LinearColourYellow : _m_LinearColourGreen);
		SetMeshColor(Edge_YZ_Z, (_m_bHoverY && _m_bHoverZ) ? _m_LinearColourYellow : _m_LinearColourBlue);

		SetMeshColor(Edge_XZ_X, (_m_bHoverX && _m_bHoverZ) ? _m_LinearColourYellow : _m_LinearColourRed);
		SetMeshColor(Edge_XZ_Z, (_m_bHoverX && _m_bHoverZ) ? _m_LinearColourYellow : _m_LinearColourBlue);

	}
}

void AGizmo::SetMeshColor(UStaticMeshComponent* MeshComp, FLinearColor NewColour)
{
	if (!MeshComp) {
		return;
	}
	if (!MeshMaterialCache.Contains(MeshComp)) {
		SetupDynamicMaterials(MeshComp);
	}
	const TArray<UMaterialInstanceDynamic*> Materials = MeshMaterialCache[MeshComp];
	for (UMaterialInstanceDynamic *DynMat : Materials ) {
		if ( DynMat ) {
			DynMat->SetVectorParameterValue(ColorVectorParamName, NewColour);
		}
	}
}

void AGizmo::SetupDynamicMaterials(UMeshComponent* MeshComp)
{
	if (!MeshComp || MeshMaterialCache.Contains(MeshComp))
		return; // Already cached

	TArray<UMaterialInstanceDynamic*> DynMats;
	const int32 MatCount = MeshComp->GetNumMaterials();

	for (int32 i = 0; i < MatCount; ++i)
	{
		UMaterialInterface* SourceMat = MeshComp->GetMaterial(i);
		if (!SourceMat) continue;

		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(SourceMat, this);
		MeshComp->SetMaterial(i, DynMat);
		DynMats.Add(DynMat);
	}

	MeshMaterialCache.Add(MeshComp, DynMats);
}

void AGizmo::NotifyParentUpdateEvent() {
	UChildActorComponent *childComp = GetParentComponent();
	if (childComp) {
		onNotifyParent.Broadcast(childComp);
	}
}