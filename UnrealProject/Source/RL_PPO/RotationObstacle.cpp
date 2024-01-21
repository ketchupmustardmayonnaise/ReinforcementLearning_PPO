// Fill out your copyright notice in the Description page of Project Settings.


#include "RotationObstacle.h"

// Sets default values
ARotationObstacle::ARotationObstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARotationObstacle::BeginPlay()
{
	Super::BeginPlay();
	speed = 0.0f;
}

// Called every frame
void ARotationObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (speed > 0.3) speed = 0.3;
	else speed += DeltaTime / 5;
	Rotate(speed);
}

void ARotationObstacle::Rotate(float rotateSpeed) {
	FRotator NewRotation = FRotator(0, rotateSpeed, 0);
	FQuat QuatRotation = FQuat(NewRotation);
	AddActorLocalRotation(QuatRotation, false, 0, ETeleportType::None);
}
