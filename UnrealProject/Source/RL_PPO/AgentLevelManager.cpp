// Fill out your copyright notice in the Description page of Project Settings.


#include "AgentLevelManager.h"

// Sets default values
AAgentLevelManager::AAgentLevelManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	currLevel = 4;
	isChangeMap = false;
}

// Called when the game starts or when spawned
void AAgentLevelManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAgentLevelManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}