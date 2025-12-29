// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "../Network/FUdpCommunicator.h"  // Your communicator header
#include "BaseActor.h"
#include "VistarGameInstance.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EVistarClassType : uint8
{
	VISTAR_TYPE_NONE        UMETA(DisplayName = "NONE"),
	VISTAR_TYPE_FIGHTER     UMETA(DisplayName = "FIGHTER"),
	VISTAR_TYPE_UAV			UMETA(DisplayName = "UAV"),
	VISTAR_TYPE_DRONE       UMETA(DisplayName = "DRONE"),
	VISTAR_TYPE_DRONE_SWARM UMETA(DisplayName = "DRONE_SWARM"),
	VISTAR_TYPE_RADAR		UMETA(DisplayName = "RADAR"),
	VISTAR_TYPE_LAUNCHER	UMETA(DisplayName = "LAUNCHER"),
	VISTAR_TYPE_MISSILE		UMETA(DisplayName = "MISSILE"),
	VISTAR_TYPE_ROUTE		UMETA(DisplayName = "ROUTE"),
};

UCLASS()
class VISTAR_API UVistarGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	void SendMessage(const FString& Message);

	void ReceiveMessage(const TSharedPtr<FJsonObject>& JsonObject);

	void UpdateVistarObject(const TSharedPtr<FJsonObject>& JsonObject, ABaseActor* baseActor, bool bRefresh);

	FVector3d LlaToUnreal(double lat, double lon, double alt,
		double refLat, double refLon, double refAlt);

	EVistarClassType GetVistarClassType(FString Str);

	UFUNCTION(BlueprintCallable, Category = "Info")
	void InitializeObjects();
	UFUNCTION(BlueprintCallable, Category = "Info")
	void Start();
	UFUNCTION(BlueprintCallable, Category = "Info")
	void Stop();

	ABaseActor* getVistarObjectById(FString sObjectId);
	ABaseActor* createNewVistarObject(FString sObjectId,FString sClass);

	UFUNCTION(BlueprintImplementableEvent, Category = "Info")
	ABaseActor* spawnVistarObjectBP(EVistarClassType eClass);

private :
	// Pointer to your communicator
	FUdpCommunicator* UdpCommunicator;


	bool _m_bRecordRefLatLongAlt;
	double _m_dRefLat, _m_dRefLon, _m_dRefAlt;

	void PopulateActorMap();

	void InitializeNetworkSendRecv();

	TMap<FString, ABaseActor*> _m_listVistarBaseActors;
};
