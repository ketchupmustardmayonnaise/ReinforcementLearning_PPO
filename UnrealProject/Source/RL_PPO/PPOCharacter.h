// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RespawnActor.h"
#include "AgentLevelManager.h"
#include "Engine/TriggerBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "PPOGoal.h"
#include "PPOCharacter.generated.h"

UCLASS()
class RL_PPO_API APPOCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APPOCharacter();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Switch Components")
	UBoxComponent* TriggerVolume;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	void AdvanceTimer();
	int resetIndex;
	UPROPERTY(EditAnywhere, Category = "Variables")
	ARespawnActor* ResetLocation;

	// count time
	FTimerHandle CountdownTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void MoveForward(float speed);
	void Rotate(float speed);

	UFUNCTION(BlueprintCallable)
	void ResetEnv();

	void Observation();
	void ApplySpeed(float DeltaTime);

	UPROPERTY(BlueprintReadWrite)
	float movementSpeed;
	float rotateSpeed;
	UPROPERTY(BlueprintReadOnly)
	float appliedMovementSpeed;
	bool jumpForward;

	TArray<float> observationCurr;
	TArray<int> OuthitActorCurr;
	float goalDistanceCurr;

	UPROPERTY(BlueprintReadOnly)
	bool isJumping;
	UPROPERTY(BlueprintReadOnly)
	int agentGround;
	bool isSendObservation;

	UPROPERTY(BlueprintReadOnly)
	bool canSelfLevelChange;


	UPROPERTY(EditAnywhere, Category = "Variables")
	AAgentLevelManager* levelManager;

	UPROPERTY(EditAnywhere, Category = "Variables")
	APPOGoal* agentGoal;

	UPROPERTY(BlueprintReadWrite)
	bool isReached;

	UPROPERTY(BlueprintReadWrite)
	bool isCollision;

	UPROPERTY(BlueprintReadWrite)
	bool isAnim;

	UPROPERTY(BlueprintReadWrite)
	bool isTimeOver;

	UPROPERTY(BlueprintReadWrite)
	bool mainreset;

	UFUNCTION(BlueprintPure)
	AAgentLevelManager* GetLevelManager();

	UFUNCTION(BlueprintPure)
	APPOGoal* GetAgentGoal();

	float jumpHeight;
	int32 CountdownTime;
	float desireTime;
	float goalAngle;
};
