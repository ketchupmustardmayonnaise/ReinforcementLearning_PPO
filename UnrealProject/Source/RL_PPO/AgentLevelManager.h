// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AgentLevelManager.generated.h"

UCLASS()
class RL_PPO_API AAgentLevelManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAgentLevelManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Variables")
	TArray<UObject*> levelMesh;

	UPROPERTY(EditAnywhere, Category = "Variables")
	TArray<FVector> relativeRespawnLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool isChangeMap;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int currLevel;
};
