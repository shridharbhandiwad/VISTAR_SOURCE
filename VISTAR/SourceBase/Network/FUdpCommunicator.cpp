// Fill out your copyright notice in the Description page of Project Settings.


#include "FUdpCommunicator.h"

FUdpCommunicator::FUdpCommunicator()
	: SenderSocket(nullptr), ReceiverSocket(nullptr), Receiver(nullptr)
{
}

FUdpCommunicator::~FUdpCommunicator()
{
	Shutdown();
}

bool FUdpCommunicator::StartSender(const FString& TargetIP, int32 TargetPort)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	RemoteAddress = SocketSubsystem->CreateInternetAddr();

	bool bIsValid;
	RemoteAddress->SetIp(*TargetIP, bIsValid);
	RemoteAddress->SetPort(TargetPort);

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid IP address"));
		return false;
	}

	SenderSocket = FUdpSocketBuilder(TEXT("UdpSenderSocket"))
		.AsReusable()
		.WithBroadcast()
		.WithSendBufferSize(2 * 1024 * 1024);

	return SenderSocket != nullptr;
}

bool FUdpCommunicator::SendMessage(const FString& Message)
{
	if (!SenderSocket || !RemoteAddress.IsValid())
	{
		return false;
	}

	FTCHARToUTF8 Utf8(*Message);
	int32 Size = Utf8.Length();
	int32 Sent = 0;

	return SenderSocket->SendTo((uint8*)Utf8.Get(), Size, Sent, *RemoteAddress) && Sent == Size;
}

bool FUdpCommunicator::StartReceiver(int32 ListenPort, FOnUdpDataReceived Callback)
{
	//FIPv4Address Addr = FIPv4Address::Any;
	//FIPv4Address::Parse(TEXT("192.168.29.58"), Addr); // Listen on all interfaces

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	FIPv4Endpoint Endpoint(FIPv4Address::LanBroadcast, ListenPort);

	ReceiverSocket = FUdpSocketBuilder(TEXT("UdpReceiverSocket"))
		.AsNonBlocking()
		.AsReusable()
		.BoundToAddress(FIPv4Address::Any)
		.BoundToPort(ListenPort)
		.WithReceiveBufferSize(2 * 1024 * 1024);


	bool bValid;

	// Join multicast group
	TSharedRef<FInternetAddr> GroupAddr = SocketSubsystem->CreateInternetAddr();
	GroupAddr->SetIp(TEXT("225.0.0.1"), bValid);
	GroupAddr->SetPort(ListenPort);

	bool bBindAll = false;
	TSharedRef<FInternetAddr> LocalInterface = SocketSubsystem->GetLocalHostAddr(*GLog, bBindAll);
	ReceiverSocket->JoinMulticastGroup(*GroupAddr);

	if (!ReceiverSocket) return false;

	ReceiverSocket->SetMulticastLoopback(true);
	Receiver = new FReceiverRunnable(ReceiverSocket, Callback);
	return true;
}

void FUdpCommunicator::Shutdown()
{
	if (SenderSocket)
	{
		SenderSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
		SenderSocket = nullptr;
	}

	if (Receiver)
	{
		Receiver->Stop();
		Receiver->Wait();
		delete Receiver;
		Receiver = nullptr;
	}
	if (ReceiverSocket)
	{
		ReceiverSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ReceiverSocket);
		ReceiverSocket = nullptr;
	}
}
