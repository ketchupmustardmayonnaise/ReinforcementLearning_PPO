// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkActor.h"

// Sets default values
ANetworkActor::ANetworkActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	resetAll = false;
}

// Called when the game starts or when spawned
void ANetworkActor::BeginPlay()
{
	Super::BeginPlay();
	OpenConnection();

	TSubclassOf<APPOCharacter> classToFind;
	TArray<AActor*> find;
	classToFind = APPOCharacter::StaticClass();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), classToFind, find);

	APPOCharacter* caster = NULL;

	for (int i = 0; i < find.Num(); i++) {
		caster = Cast<APPOCharacter>(find[i]);
		agents.Add(caster);
	}

	if (isExistMain) {
		for (int i = 0; i < agents.Num(); i++) {
			agents[i]->canSelfLevelChange = false;
		}
	}

	HasInput.Init(false, agents.Num());
	AppliedInput.Init(false, agents.Num());
	Input.Init(0, agents.Num() * 3);
}

void ANetworkActor::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	Close_Connection();
};

// Called every frame
void ANetworkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (resetAll) {
		for (int i = 0; i < agents.Num(); i++) {
			if (!agents[i]->ActorHasTag("main")) {
				agents[i]->levelManager->isChangeMap = true;
				agents[i]->ResetEnv();
			}
		}
		resetAll = false;
	}

	for (int id = 0; id < agents.Num(); id++)
	{
		ManageConnection();
		if (Input[id] == 10)
		{
			if (!agents[id]->ActorHasTag("main") || agents[id]->isAnim == false) {
				agents[id]->ResetEnv();
			}
		}
		else if (Input[id] == 11)
		{

		}
		else
		{
			agents[id]->ApplySpeed(DeltaTime);
		}

		// ======

		if (Input[id] != 10 && AppliedInput[id]) {
			AppliedInput[id] = false;
			SendObservation(id);
		}

		if (Input[id] == 10)
		{
			Input[id] = 11;
		}

		if (HasInput[id]) HasInput[id] = false;
	}
}

void ANetworkActor::OpenConnection()
{
	if (!IsConnectionOpen) {
		UE_LOG(LogTemp, Warning, TEXT("Open Connection"));
		IsConnectionOpen = true;
		WaitingForConnection = true;

		FIPv4Address IPAddress;
		FIPv4Address::Parse(FString("127.0.0.1"), IPAddress);
		FIPv4Endpoint Endpoint(IPAddress, (uint16)5050);

		ListenSocket = FTcpSocketBuilder(TEXT("TcpSocket")).AsReusable();

		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		ListenSocket->Bind(*SocketSubsystem->CreateInternetAddr(Endpoint.Address.Value, Endpoint.Port));
		ListenSocket->Listen(1);
		UE_LOG(LogTemp, Warning, TEXT("Listening"))
	}
}

void ANetworkActor::Close_Connection()
{
	if (IsConnectionOpen) {
		UE_LOG(LogTemp, Warning, TEXT("Closing Connection"));
		IsConnectionOpen = false;

		ListenSocket->Close();
	}
}

void ANetworkActor::ManageConnection()
{
	if (WaitingForConnection) {
		TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		bool hasConnection = false;
		if (ListenSocket->HasPendingConnection(hasConnection) && hasConnection) {
			ConnectionSocket = ListenSocket->Accept(*RemoteAddress, TEXT("Connection"));
			WaitingForConnection = false;
			UE_LOG(LogTemp, Warning, TEXT("Incoming connection"));
			// start Recv thread
			ClientConnectionFinishedFuture = Async(EAsyncExecution::LargeThreadPool, [&]() {
				UE_LOG(LogTemp, Warning, TEXT("recv thread started"));
				ReceiveArrayMessages();
				}
			);
		}
	}
}

void ANetworkActor::ReceiveArrayMessages()
{
	while (IsConnectionOpen) {
		uint32 size;
		TArray<uint8> ReceivedData;

		if (ConnectionSocket->HasPendingData(size)) {
			ReceivedData.Init(0, 1024);
			int32 Read = 0;
			ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
			if (ReceivedData.Num() > 0)
			{
				TArray<float> ReceivedArray;
				const int32 numElements = ReceivedData.Num() / sizeof(float);
				ReceivedArray.SetNum(numElements);
				FMemory::Memcpy(ReceivedArray.GetData(), ReceivedData.GetData(), ReceivedArray.Num());

				//// ==== Array Receive ====
				int _id = int(ReceivedArray[0]);
				if (ReceivedArray[1] == 10) {
					Input[_id] = 10;
				}
				else {
					Input[_id] = int(ReceivedArray[1]);
				}
				HasInput[_id] = true;

				// Input 적용
				if (HasInput[_id]) {
					AppliedInput[_id] = true;
					if (Input[_id] == 10) {
					}
					else {
						if (agents[_id]->isAnim == false) {
							if (Input[_id] == 1)
							{
								agents[_id]->movementSpeed = 10;
								if (agents[_id]->rotateSpeed >= 2) agents[_id]->rotateSpeed -= 2;
								else if (agents[_id]->rotateSpeed <= 2) agents[_id]->rotateSpeed += 2;
								agents[_id]->jumpForward = false;
							}
							else if (Input[_id] == 0) {
								if(agents[_id]->movementSpeed >= 4) agents[_id]->movementSpeed -= 4;
								if (agents[_id]->rotateSpeed >= 2) agents[_id]->rotateSpeed -= 2;
								else if (agents[_id]->rotateSpeed <= 2) agents[_id]->rotateSpeed += 2;
								agents[_id]->jumpForward = false;
							}
							else if (Input[_id] == 2) {
								if (agents[_id]->movementSpeed >= 4) agents[_id]->movementSpeed -= 4;
								agents[_id]->rotateSpeed = -2;
								agents[_id]->jumpForward = false;
							}
							else if (Input[_id] == 3) {
								if (agents[_id]->movementSpeed >= 4) agents[_id]->movementSpeed -= 4;
								agents[_id]->rotateSpeed = 2;
								agents[_id]->jumpForward = false;
							}
							else if (Input[_id] == 4) { // jump
								if (agents[_id]->movementSpeed >= 4) agents[_id]->movementSpeed -= 4;
								agents[_id]->jumpForward = true;
								if (agents[_id]->rotateSpeed >= 2) agents[_id]->rotateSpeed -= 2;
								else if (agents[_id]->rotateSpeed <= 2) agents[_id]->rotateSpeed += 2;
							}
						}
					}
				}
			}
		}
	}
}

void ANetworkActor::SendObservation(int id)
{
	// 보낼 배열
	agents[id]->Observation();
	TArray<float> observationCurr = agents[id]->observationCurr;
	TArray<int> OuthitActorCurr = agents[id]->OuthitActorCurr;

	TArray<float> NewState = {
		(std::round(float(OuthitActorCurr[0])) == 1 ? std::round(observationCurr[0]) : 25.0f),
		(std::round(float(OuthitActorCurr[0])) == 2 ? std::round(observationCurr[0]) : 25.0f),

		(std::round(float(OuthitActorCurr[1])) == 1 ? std::round(observationCurr[1]) : 25.0f),
		(std::round(float(OuthitActorCurr[1])) == 2 ? std::round(observationCurr[1]) : 25.0f),

		(std::round(float(OuthitActorCurr[2])) == 1 ? std::round(observationCurr[2]) : 25.0f),
		(std::round(float(OuthitActorCurr[2])) == 2 ? std::round(observationCurr[2]) : 25.0f),

		(std::round(float(OuthitActorCurr[3])) == 1 ? std::round(observationCurr[3]) : 25.0f),
		(std::round(float(OuthitActorCurr[3])) == 2 ? std::round(observationCurr[3]) : 25.0f),

		(std::round(float(OuthitActorCurr[4])) == 1 ? std::round(observationCurr[4]) : 25.0f),
		(std::round(float(OuthitActorCurr[4])) == 2 ? std::round(observationCurr[4]) : 25.0f),

		(std::round(float(OuthitActorCurr[5])) == 1 ? std::round(observationCurr[5]) : 25.0f),
		(std::round(float(OuthitActorCurr[5])) == 2 ? std::round(observationCurr[5]) : 25.0f),

		(std::round(float(OuthitActorCurr[6])) == 1 ? std::round(observationCurr[6]) : 25.0f),
		(std::round(float(OuthitActorCurr[6])) == 2 ? std::round(observationCurr[6]) : 25.0f),

		(std::round(float(OuthitActorCurr[7])) == 1 ? std::round(observationCurr[7]) : 25.0f),
		(std::round(float(OuthitActorCurr[7])) == 2 ? std::round(observationCurr[7]) : 25.0f),

		(std::round(float(OuthitActorCurr[8])) == 1 ? std::round(observationCurr[8]) : 25.0f),
		(std::round(float(OuthitActorCurr[8])) == 2 ? std::round(observationCurr[8]) : 25.0f),

		(std::round(float(OuthitActorCurr[9])) == 1 ? std::round(observationCurr[9]) : 25.0f),
		(std::round(float(OuthitActorCurr[9])) == 2 ? std::round(observationCurr[9]) : 25.0f),

		(std::round(float(OuthitActorCurr[10])) == 1 ? std::round(observationCurr[10]) : 25.0f),
		(std::round(float(OuthitActorCurr[10])) == 2 ? std::round(observationCurr[10]) : 25.0f),

		(std::round(float(OuthitActorCurr[11])) == 1 ? std::round(observationCurr[11]) : 25.0f),
		(std::round(float(OuthitActorCurr[11])) == 2 ? std::round(observationCurr[11]) : 25.0f),

		(std::round(float(OuthitActorCurr[12])) == 1 ? std::round(observationCurr[12]) : 25.0f),
		(std::round(float(OuthitActorCurr[12])) == 2 ? std::round(observationCurr[12]) : 25.0f),

		(std::round(float(OuthitActorCurr[13])) == 1 ? std::round(observationCurr[13]) : 25.0f),
		(std::round(float(OuthitActorCurr[13])) == 2 ? std::round(observationCurr[13]) : 25.0f),

		(std::round(float(OuthitActorCurr[14])) == 1 ? std::round(observationCurr[14]) : 25.0f),
		(std::round(float(OuthitActorCurr[14])) == 2 ? std::round(observationCurr[14]) : 25.0f),

		(std::round(float(OuthitActorCurr[15])) == 1 ? std::round(observationCurr[15]) : 25.0f),
		(std::round(float(OuthitActorCurr[15])) == 2 ? std::round(observationCurr[15]) : 25.0f),

		std::max(std::min(1.0f, (agents[id]->CountdownTime - agents[id]->desireTime) / agents[id]->desireTime), -1.0f),
		std::round(agents[id]->goalDistanceCurr),
		std::round(agents[id]->goalAngle),

		std::min(4.0f, agents[id]->appliedMovementSpeed),
		(agents[id]->isJumping ? 1.0f : 0.0f),
		(agents[id]->isCollision ? 1.0f : 0.0f),
		float(agents[id]->agentGround),
		(agents[id]->isReached? 1.0f : (agents[id]->isTimeOver ? -1.0f : 0.0f))
		};

	agents[id]->isSendObservation = true;
	agents[id]->isCollision = false;

	uint32 ArraySize = NewState.Num();
	int32 sent = 0;
	ConnectionSocket->Send(reinterpret_cast<const uint8*>(&ArraySize), sizeof(uint32), sent);

	TArray<uint8> ArrayData;
	ArrayData.SetNumUninitialized(ArraySize * sizeof(float));
	FMemory::Memcpy(ArrayData.GetData(), NewState.GetData(), ArrayData.Num());
	sent = 0;
	ConnectionSocket->Send(ArrayData.GetData(), ArrayData.Num(), sent);
}