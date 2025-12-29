// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Networking.h"
\
/**
 * 
 */

DECLARE_DELEGATE_OneParam(FOnUdpDataReceived, const TSharedPtr<FJsonObject>& /*Data*/);

class VISTAR_API FUdpCommunicator
{
public:
	FOnUdpDataReceived OnDataReceived;

private:
	class FReceiverRunnable : public FRunnable
	{
	public:
		FReceiverRunnable(FSocket* InSocket, FOnUdpDataReceived InCallback)
			: Socket(InSocket), bStop(false), OnDataReceived(InCallback)
		{
			Thread = FRunnableThread::Create(this, TEXT("FReceiverRunnable"));
		}
		virtual ~FReceiverRunnable()
		{
			if (Thread) { Thread->Kill(true); delete Thread; }
		}
		virtual uint32 Run() override
		{
			//TArray<uint8> Buffer;
			//Buffer.SetNumUninitialized(1024);
			char Buffer[10000] = { 0 };
			while (!bStop)
			{
				uint32 Pending = 0;
				if (Socket && Socket->HasPendingData(Pending) && Pending > 0)
				{

					//Buffer.SetNumUninitialized(FMath::Min(Pending, 65507u));
					int32 Read = 0;
					if (Socket->Recv(reinterpret_cast<uint8*>(Buffer), sizeof(Buffer), Read) && Read > 0)
					{
						int32 size = sizeof(Buffer) - 1;
						Buffer[FMath::Min(Read, size)] = '\0';
						FString ReceivedString = FString(UTF8_TO_TCHAR(Buffer));
						// Convert bytes to FString (assuming UTF8)
						//FUTF8ToTCHAR Converter(reinterpret_cast<const ANSICHAR*>(Buffer.GetData()), Read);
						//FString ReceivedString(Converter.Get(), Converter.Length());
						//FString ReceivedString = FString(UTF8_TO_TCHAR(Buffer.GetData()));

						if (OnDataReceived.IsBound())
						{
							//FString AsString;
							//TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&AsString);
							//FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

							//const char* JsonData = reinterpret_cast<const char*>(Buffer);
							TSharedPtr<FJsonObject> JsonObject;
							TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(ReceivedString);
							bool bValid = FJsonSerializer::Deserialize(Reader, JsonObject);
							if (bValid) {
								// Call delegate on game thread asynchronously
								//AsyncTask(ENamedThreads::GameThread, [JsonObject, Callback = OnDataReceived]()
									//{
								UE_LOG(LogTemp, Log, TEXT("Recvd JSON string!"));
								OnDataReceived.Execute(JsonObject);//UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON string!"));
									//});
							}
							else {
								UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON string!"));
							}
						}
					}
				}
				else
				{
					FPlatformProcess::Sleep(0.01f);
				}
			}
			return 0;
		}
		virtual void Stop() override { bStop = true; }
		void Wait() { if (Thread) Thread->WaitForCompletion(); }

	private:
		FSocket* Socket;
		FRunnableThread* Thread;
		FThreadSafeBool bStop;
		// Delegate to bind your callback
		FOnUdpDataReceived OnDataReceived;
	};


public:
	FUdpCommunicator();
	~FUdpCommunicator();

	// Initialize sender
	bool StartSender(const FString& TargetIP, int32 TargetPort);

	// Initialize receiver
	bool StartReceiver(int32 ListenPort, FOnUdpDataReceived Callback);

	// Send a message
	bool SendMessage(const FString& Message);

	// Cleanup
	void Shutdown();

private:
	FSocket* SenderSocket;
	FSocket* ReceiverSocket;
	FReceiverRunnable *Receiver;
	TSharedPtr<FInternetAddr> RemoteAddress;
};
