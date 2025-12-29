// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseActor.generated.h"

class UWidgetComponent;
class AActor;
UCLASS()
class VISTAR_API ABaseActor : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static void InitActor();

	// Root component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* WidgetObjectIdComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Info")
	FString sObjectId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString sObjectClass = "OBJECT";

	UFUNCTION(BlueprintImplementableEvent, Category = "Info")
	void OnObjectIdGenerated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Info")
	void OnObjectDestroyed();

	UFUNCTION(BlueprintCallable, Category = "Info")
	FString GetObjectId();

	UFUNCTION(BlueprintCallable, Category = "Info")
	double GetSlewAz();

	UFUNCTION(BlueprintCallable, Category = "Info")
	double GetSlewElev();

	void SetObjectId(FString objectId);

	void UpdatePositionXYZ(double X, double Y, double Z);

	void UpdateRotationYPR(double Yaw, double Pitch, double Roll);

	void UpdateSlew(double slewAz, double slewElev);

	void setParentInfo(FString ParentId, int childId);

	UFUNCTION(BlueprintImplementableEvent, Category = "Info")
	void attachChildtoSocket(AActor* childActor, const FString &sSocketName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Info")
	void activateOnUpdate();

	void unsetParentInfo();

	void Refresh();

	virtual void TransmitSelfInfo() {};

	void ProcessAction(FString sAction);


private :

	double _m_dPosX;
	double _m_dPosY;
	double _m_dPosZ;

	double _m_dRotYaw;
	double _m_dRotPitch;
	double _m_dRotRoll;

	double _m_dSlewAz;
	double _m_dSlewElev;

	FString sParentId;
	int _m_nChildId;

	bool _m_bRefresh;

};
