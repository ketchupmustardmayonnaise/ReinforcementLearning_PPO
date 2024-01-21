// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking.h"
#include "Async/Async.h"
#include "Sockets.h"
#include "Kismet/GameplayStatics.h"
#include "PPOCharacter.h"
#include "AgentLevelManager.h"
#include "vector"
#include "NetworkActor.generated.h"

UCLASS()
class RL_PPO_API ANetworkActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetworkActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	TArray<bool> HasInput;
	TArray<bool> AppliedInput;
	// input을 배열 형식으로 처리해야 할 것 같음.
	TArray<int> Input;
	TArray<AAgentLevelManager*> levelManagers;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	bool IsConnectionOpen = false;
	bool WaitingForConnection = false;
	void OpenConnection();
	void Close_Connection();
	FSocket* ListenSocket = NULL;
	TArray<APPOCharacter*> agents;

	void ManageConnection();
	void ReceiveArrayMessages();
	void SendObservation(int id);
	FSocket* ConnectionSocket = NULL;
	TFuture<void> ClientConnectionFinishedFuture;

	UPROPERTY(EditAnywhere, Category = "Variables")
	APPOCharacter* mainCharacter;

	UPROPERTY(BlueprintReadWrite)
	bool resetAll;

	UPROPERTY(EditAnywhere, Category = "Variables")
	bool isExistMain;
};
