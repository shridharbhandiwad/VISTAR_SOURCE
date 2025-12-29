// Fill out your copyright notice in the Description page of Project Settings.


#include "VistarGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UVistarGameInstance::Init()
{
    _m_bRecordRefLatLongAlt = false;
    Super::Init();
    ABaseActor::InitActor();
    _m_listVistarBaseActors.Empty();
    _m_bRecordRefLatLongAlt = false;
    //PopulateActorMap();
    InitializeNetworkSendRecv();

    //FVector3d Vec1 = LlaToUnreal(13.0, 77, 0,
    //    13, 77, 0);

    //FVector3d Vec2 = LlaToUnreal(14.00000, 77, 0,
    //    13, 77, 0);

    //UE_LOG(LogTemp, Warning, TEXT("Vec1: X=%f Y=%f Z=%f"), Vec1.X, Vec1.Y, Vec1.Z);
    //UE_LOG(LogTemp, Warning, TEXT("Vec2: X=%f Y=%f Z=%f"), Vec2.X, Vec2.Y, Vec2.Z);

    //int a = 1;

}

void UVistarGameInstance::Shutdown()
{
    if (UdpCommunicator)
    {
        UdpCommunicator->Shutdown();
        delete UdpCommunicator;
        UdpCommunicator = nullptr;
    }

    Super::Shutdown();
}

void UVistarGameInstance::PopulateActorMap()
{
    //_m_listVistarBaseActors.Empty(); // Clear existing entries

    //UWorld* World = GetWorld();
    //if (!World) return;

    //TArray<AActor*> FoundActors;
    //UGameplayStatics::GetAllActorsOfClass(World, ABaseActor::StaticClass(), FoundActors);

    //for (AActor* Actor : FoundActors)
    //{
    //    ABaseActor* BaseActor = Cast<ABaseActor>(Actor);
    //    if (BaseActor)
    //    {
    //        FString Key = BaseActor->GetObjectId(); // You can use a custom ID instead
    //        _m_listVistarBaseActors.Add(Key, BaseActor);
    //    }
    //}
}

void UVistarGameInstance::InitializeNetworkSendRecv() {

    UdpCommunicator = new FUdpCommunicator();

    FOnUdpDataReceived Callback;
    Callback.BindUObject(this, &UVistarGameInstance::ReceiveMessage);

    bool bStarted = UdpCommunicator->StartReceiver(8888, Callback);  // Example port

    if (!bStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to start UDP receiver!"));
        delete UdpCommunicator;
        UdpCommunicator = nullptr;
    }
    else
    {
        bStarted = UdpCommunicator->StartSender("255.0.0.1", 7777);
        if (!bStarted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to start UDP Sender!"));
        }
        UE_LOG(LogTemp, Log, TEXT("UDP receiver started successfully."));
    }
}

void UVistarGameInstance::SendMessage(const FString& Message)
{
    if (UdpCommunicator) {
        UdpCommunicator->SendMessage(Message);
    }
}


FVector3d UVistarGameInstance::LlaToUnreal(double lat, double lon, double alt,
    double refLat, double refLon, double refAlt)
{
    auto LLA2ECEF = [](double latDeg, double lonDeg, double altM)
        {
            double a = 6378137.0;
            double e2 = 6.69437999014e-3;

            double lat = FMath::DegreesToRadians(latDeg);
            double lon = FMath::DegreesToRadians(lonDeg);

            double sinLat = sin(lat);
            double cosLat = cos(lat);
            double sinLon = sin(lon);
            double cosLon = cos(lon);

            double N = a / sqrt(1 - e2 * sinLat * sinLat);

            double x = (N + altM) * cosLat * cosLon;
            double y = (N + altM) * cosLat * sinLon;
            double z = (N * (1 - e2) + altM) * sinLat;

            return FVector3d(x, y, z);
        };

    FVector3d ecef0 = LLA2ECEF(refLat, refLon, refAlt);
    FVector3d ecef1 = LLA2ECEF(lat, lon, alt);

    FVector3d d = ecef1 - ecef0;

    double lat0 = FMath::DegreesToRadians(refLat);
    double lon0 = FMath::DegreesToRadians(refLon);

    double sinLat = sin(lat0), cosLat = cos(lat0);
    double sinLon = sin(lon0), cosLon = cos(lon0);

    FVector3d east(-sinLon, cosLon, 0);
    FVector3d north(-sinLat * cosLon, -sinLat * sinLon, cosLat);
    FVector3d up(cosLat * cosLon, cosLat * sinLon, sinLat);

    return FVector3d(
        east.Dot(d) * 100.0,     // X east
        north.Dot(d) * 100.0,    // Y north
        up.Dot(d) * 100.0 - 5250.0     // Z up
    );
}

void UVistarGameInstance::ReceiveMessage(const TSharedPtr<FJsonObject>& JsonObject)
{
    FString sStream = JsonObject->GetStringField("STREAM");
    FString sId = JsonObject->GetStringField("ID");
    FString sClass = JsonObject->GetStringField("CLASS");
    if (sClass.Equals("route")) {
        return;
    }

    ABaseActor* baseActor = getVistarObjectById(sId);
    if (IsValid(baseActor)) {
        if (sStream.Equals("create") || sStream.Equals("update")) {
            UpdateVistarObject(JsonObject, baseActor, true);
        }
        else if (sStream.Equals("delete")) {
            AsyncTask(ENamedThreads::GameThread, [this, sId,baseActor]()
                {
                    _m_listVistarBaseActors.Remove(sId);
                    baseActor->Reset();
                    baseActor->Destroy();
                });
        }
        else {
            FString sAction = JsonObject->GetStringField("ACTION");
            baseActor->ProcessAction(sAction);
        }
    }
    else {
        AsyncTask(ENamedThreads::GameThread, [this, sId, sClass, sStream,JsonObject]()
            {
                ABaseActor* newActor = createNewVistarObject(sId, sClass);
                if (newActor) {
                    bool bRefresh = true;
                    if (sStream.Equals("create")) {
                        FString ParentId = JsonObject->GetStringField("PARENT");
                        int childId = JsonObject->GetNumberField("CHILD_ID");
                        newActor->setParentInfo(ParentId, childId);
                        if (!ParentId.IsEmpty()) {
                            ABaseActor* parentActor = getVistarObjectById(ParentId);
                            if (parentActor) {
                                newActor->setParentInfo(ParentId, childId);
                                FString sSocketId = FString::Printf(TEXT("Child_%d"), childId);
                                parentActor->attachChildtoSocket(newActor, sSocketId);
                            }
                            
                            bRefresh = false;
                        }
                    }
                    UpdateVistarObject(JsonObject, newActor, bRefresh);
                }
            });
    }
}

void UVistarGameInstance::UpdateVistarObject(const TSharedPtr<FJsonObject>& JsonObject,ABaseActor* baseActor, bool bRefresh) {

    TSharedPtr<FJsonObject> jsonLocation = JsonObject->GetObjectField("LOCATION");

    double dLon = static_cast<double>(FCString::Atof(*jsonLocation->GetStringField("X")));
    double dLat = static_cast<double>(FCString::Atof(*jsonLocation->GetStringField("Y")));
    double dAlt = static_cast<double>(FCString::Atof(*jsonLocation->GetStringField("Z")));

    if (!_m_bRecordRefLatLongAlt) {
        _m_bRecordRefLatLongAlt = true;
        _m_dRefLon = dLon;
        _m_dRefLat = dLat;
        _m_dRefAlt = 0;
    }

    FVector3d vectorXYZ = LlaToUnreal(dLon, dLat, dAlt, _m_dRefLon, _m_dRefLat, _m_dRefAlt);

    double X = vectorXYZ.X;
    double Y = vectorXYZ.Y;
    double Z = vectorXYZ.Z;


    TSharedPtr<FJsonObject> jsonRotation = JsonObject->GetObjectField("ROTATION");

    double Yaw = static_cast<double>(FCString::Atof(*jsonRotation->GetStringField("YAW")));
    double Pitch = static_cast<double>(FCString::Atof(*jsonRotation->GetStringField("PITCH")));
    double Roll = static_cast<double>(FCString::Atof(*jsonRotation->GetStringField("ROLL")));

    TSharedPtr<FJsonObject> jsonSlew = JsonObject->GetObjectField("SLEW");

    double SlewAz = static_cast<double>(FCString::Atof(*jsonSlew->GetStringField("SLEW_AZ")));
    double SlewElev = static_cast<double>(FCString::Atof(*jsonSlew->GetStringField("SLEW_ELEV")));

    if (IsValid(baseActor)) {
        baseActor->UpdatePositionXYZ(X,Y,Z);
        baseActor->UpdateRotationYPR(Yaw,Pitch,Roll);
        baseActor->UpdateSlew(SlewAz, SlewElev);
        if (bRefresh) {
            baseActor->unsetParentInfo();
            baseActor->Refresh();
        }
    }
}

ABaseActor* UVistarGameInstance::getVistarObjectById(FString sObjectId) {
    if (ABaseActor** baseActorPtr = _m_listVistarBaseActors.Find(sObjectId)) {
        if (baseActorPtr != nullptr) {
            ABaseActor* baseActor = *baseActorPtr;
            return baseActor;
        }
    }
    return nullptr;
}

ABaseActor* UVistarGameInstance::createNewVistarObject(FString sObjectId,FString sClass) {

    ABaseActor* actor = spawnVistarObjectBP(GetVistarClassType(sClass));
    if (actor) {
        actor->SetObjectId(sObjectId);
        _m_listVistarBaseActors.Add(sObjectId, actor);
    }
    return actor;
}

EVistarClassType UVistarGameInstance::GetVistarClassType(FString Str)
{
    if (Str.Equals(TEXT("drone")))               return EVistarClassType::VISTAR_TYPE_DRONE;
    else if (Str.Equals(TEXT("drone_swarm")))    return EVistarClassType::VISTAR_TYPE_DRONE_SWARM;
    else if (Str.Equals(TEXT("fighter")))        return EVistarClassType::VISTAR_TYPE_FIGHTER;
    else if (Str.Equals(TEXT("uav")))            return EVistarClassType::VISTAR_TYPE_UAV;
    else if (Str.Equals(TEXT("radar")))          return EVistarClassType::VISTAR_TYPE_RADAR;
    else if (Str.Equals(TEXT("launcher")))       return EVistarClassType::VISTAR_TYPE_LAUNCHER;
    else if (Str.Equals(TEXT("missile")))        return EVistarClassType::VISTAR_TYPE_MISSILE;
    else if (Str.Equals(TEXT("route")))          return EVistarClassType::VISTAR_TYPE_ROUTE;
    else                                         return EVistarClassType::VISTAR_TYPE_NONE;
}

void UVistarGameInstance::InitializeObjects()
{
    PopulateActorMap();
    for (const TPair<FString, ABaseActor*>& Elem : _m_listVistarBaseActors)
    {
        const FString& Key = Elem.Key;
        ABaseActor* baseActor = Elem.Value;

        if (baseActor)
        {
            baseActor->TransmitSelfInfo();
        }
    }
}

void UVistarGameInstance::Start()
{
    FString JsonOutput;

    TSharedRef<FJsonObject> JsonObjectRoot = MakeShared<FJsonObject>();

    // Add key-value pairs
    JsonObjectRoot->SetStringField(TEXT("STREAM"), TEXT("Event"));
    JsonObjectRoot->SetStringField(TEXT("TYPE"), TEXT("Start"));

    // Convert to string
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonOutput);
    FJsonSerializer::Serialize(JsonObjectRoot, Writer);

    SendMessage(JsonOutput);
}

void UVistarGameInstance::Stop()
{
    FString JsonOutput;

    TSharedRef<FJsonObject> JsonObjectRoot = MakeShared<FJsonObject>();

    // Add key-value pairs
    JsonObjectRoot->SetStringField(TEXT("STREAM"), TEXT("Event"));
    JsonObjectRoot->SetStringField(TEXT("TYPE"), TEXT("Stop"));

    // Convert to string
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonOutput);
    FJsonSerializer::Serialize(JsonObjectRoot, Writer);

    SendMessage(JsonOutput);
}