// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseActor.h"
#include "Components/WidgetComponent.h"

// Sets default values
ABaseActor::ABaseActor()
{
	// Create and set the root component
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create and attach the widget component
	WidgetObjectIdComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetObjectIdComponent"));
	WidgetObjectIdComponent->SetupAttachment(RootComponent);  // Attach to root or other component

	//WidgetObjectIdComponent->SetDrawSize(FVector2D(200.f, 100.f));
	//WidgetObjectIdComponent->SetWidgetSpace(EWidgetSpace::Screen); // Or World

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseActor::BeginPlay()
{
	Super::BeginPlay();

	FVector loc = GetActorLocation();
	_m_dPosX = loc.X;
	_m_dPosY = loc.Y;
	_m_dPosZ = loc.Z;

	_m_dRotYaw = 0.0;
	_m_dRotPitch = 0.0;
	_m_dRotRoll = 0.0;

	_m_dSlewAz = 0.0;
	_m_dSlewElev = 0.0;

	_m_nChildId = 0;
}

// Called every frame
void ABaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (_m_bRefresh) {
		_m_bRefresh = false;
		SetActorLocation(FVector(_m_dPosX, _m_dPosY, _m_dPosZ));

		//FQuat Quat = FQuat::MakeFromEuler(FVector(_m_dRotPitch, FMath::Fmod(_m_dRotYaw + 360.f, 360.f), _m_dRotRoll));
		//SetActorRotation(Quat);

		SetActorRotation(FRotator(_m_dRotPitch, FMath::Fmod(_m_dRotYaw + 360.f, 360.f), _m_dRotRoll));
	}
}
void ABaseActor::InitActor() {

}

FString ABaseActor::GetObjectId() {
	return sObjectId;
}

void ABaseActor::SetObjectId(FString objectId) {
	sObjectId = objectId;
	OnObjectIdGenerated();
}


void ABaseActor::UpdatePositionXYZ( double X, double Y, double Z ) {
	_m_dPosX = X;
	_m_dPosY = Y;
	_m_dPosZ = Z;
}

void ABaseActor::UpdateRotationYPR(double Yaw, double Pitch, double Roll) {
	_m_dRotYaw = Yaw;
	_m_dRotPitch = Pitch;
	_m_dRotRoll = Roll;
	UE_LOG(LogTemp, Warning, TEXT("YPR : Y=%f P=%f R=%f"), _m_dRotYaw, _m_dRotPitch, _m_dRotRoll);
}

void ABaseActor::UpdateSlew(double slewAz, double slewElev) {
	_m_dSlewAz = slewAz;
	_m_dSlewElev = slewElev;
}


void ABaseActor::setParentInfo(FString ParentId, int childId) {
	_m_nChildId = childId;
	if (_m_nChildId > 0) {
		WidgetObjectIdComponent->SetVisibility(false);
	}
}

void ABaseActor::unsetParentInfo() {
	if (_m_nChildId > 0) {

	AsyncTask(ENamedThreads::GameThread, [this]()
		{
			DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			_m_nChildId = 0;
			WidgetObjectIdComponent->SetVisibility(true);
			activateOnUpdate();
		});
	}
}


double ABaseActor::GetSlewAz() {
	return _m_dSlewAz;
}

double ABaseActor::GetSlewElev() {
	return _m_dSlewElev;
}

void ABaseActor::Refresh() {
	_m_bRefresh = true;
}

void ABaseActor::ProcessAction(FString sAction) {
	if (sAction.Contains("destroy")) {
		AsyncTask(ENamedThreads::GameThread, [this]()
			{
				OnObjectDestroyed();
			});
	}
}
