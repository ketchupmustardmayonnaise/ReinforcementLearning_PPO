// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnActor.h"

// Sets default values
ARespawnActor::ARespawnActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARespawnActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARespawnActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector2D ARespawnActor::Location() {
	return FVector2D(GetActorLocation().X, GetActorLocation().Y);
}
FVector ARespawnActor::Location3D() {
	return FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
}