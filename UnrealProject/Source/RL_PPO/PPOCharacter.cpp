// Fill out your copyright notice in the Description page of Project Settings.


#include "PPOCharacter.h"
#include "Math/Vector.h"

// Sets default values
APPOCharacter::APPOCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CountdownTime = 180;
	movementSpeed = 0;
	rotateSpeed = 0;
	jumpHeight = 170;
	appliedMovementSpeed = 0;
	jumpForward = false;
	resetIndex = 0;
	isSendObservation = false;
	canSelfLevelChange = true;
	isAnim = false;
	mainreset = false;

	observationCurr.Init(25, 11);
	goalDistanceCurr = 25;
	OuthitActorCurr.Init(0, 11);
}

// Called when the game starts or when spawned
void APPOCharacter::BeginPlay()
{
	Super::BeginPlay();
	ResetEnv();
	
	// jump setting
	JumpMaxCount = 1;
	GetCharacterMovement()->JumpZVelocity = 650.f;

	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this,
		&APPOCharacter::AdvanceTimer, 1.0f, true);
	resetIndex = 0;
}

// Called every frame
void APPOCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// jump
	if (jumpForward) {
		Jump();
		isJumping = true;
	}
	if (mainreset) {
		ResetEnv();
		mainreset = false;
	}
	MoveForward(movementSpeed);
	Rotate(rotateSpeed);

	//GetMesh()->SetSimulatePhysics
	//DownVec
	FHitResult OutHit;
	FVector Start = this->GetActorLocation() + FVector(0, 0, -30);
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	FVector DownVec = FVector(0, 0, -1) * 700.f + Start;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, DownVec, ECC_EngineTraceChannel4, CollisionParams)) {
		if (OutHit.GetComponent()->GetName() == "obstacle") agentGround = 4;
		isSendObservation = false;
	}
	appliedMovementSpeed = std::round(sqrt(GetVelocity().X * GetVelocity().X + GetVelocity().Y * GetVelocity().Y) / 100);
}

// Called to bind functionality to input
void APPOCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void APPOCharacter::MoveForward(float speed) {
	FVector forwardVec = this->GetActorForwardVector() * speed;
	AddMovementInput(forwardVec);
}


void APPOCharacter::Rotate(float speed) {
	FRotator NewRotation = FRotator(0, speed, 0);
	FQuat QuatRotation = FQuat(NewRotation);
	AddActorLocalRotation(QuatRotation, false, 0, ETeleportType::None);
}


void APPOCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) {
	if (Other != this && Other->ActorHasTag("floor"))
	{
		isJumping = false;
		jumpForward = false;
	}
	if (Other != this && Other->ActorHasTag("wall"))
	{
		isCollision = true;
	}
	if (Other != this && Other->ActorHasTag("obstacle"))
	{
		isCollision = true;
	}
	if (Other != this && Other->ActorHasTag("stair") && agentGround == 0) {
		isCollision = true;
	}
}


void APPOCharacter::ResetEnv() {
	this->SetActorRotation(FRotator(0.0f, 90.0f, 0.0f));
	int x = FMath::RandRange(-1190, 1200);
	int y = FMath::RandRange(-1000, 1000);
	int z = 100;
	if (levelManager->currLevel == 2) {
		if (-220 < y && y < 225) y = -220;
	}
	else if (levelManager->currLevel == 3) {
		if (149 < y && y < 525) y = 149;
		else if (-445 < y && y < 45) y = -445;
	}
	else if (levelManager->currLevel == 4) {
		if (39 < y && y < 225) y = 39;
	}
	else if (levelManager->currLevel == 5) {
		if (-865 < y && y < -475) y = -865;
		else if (-75 < y && y < 305) y = -75;
		if (380 < x && x < 590) x = 590;
	}
	else if (levelManager->currLevel == 6) {
		if (-150 < y && y < 150) y = ((int(FMath::RandRange(0, 1))) == 0? -150 : 150);
		
		if (y <= -150) x = FMath::RandRange(-250, 1200);
		else if (y >= 150) x = FMath::RandRange(-1190, 250);
	}
	else if (levelManager->currLevel == 7) {
		if (-390 < x && x < 850 && -630 < y && y < -21) z = 230;
		else if (-1070 < x && x < 1010 && -21 < y && y < 549) z = 330;
		else if (-720 < x && x < 590 && 549 < y) z = 455;
	}
	
	this->SetActorLocation(FVector
	(levelManager->GetActorLocation().X + x,
		levelManager->GetActorLocation().Y + y,
		levelManager->GetActorLocation().Z + z));


	isReached = false;
	isCollision = false;
	jumpForward = false;
	isTimeOver = false;
	desireTime = std::min(25.0f, GetDistanceTo(agentGoal) / 100) * 3 + 5;

	// count time
	CountdownTime = desireTime * 2;
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this,
		&APPOCharacter::AdvanceTimer, 1.0f, true);
}


void APPOCharacter::Observation() {
	FHitResult OutHit;
	FVector Start = this->GetActorLocation() + FVector(0, 0, -30);
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	TArray<int> detectGoal;
	
	// 초기화
	observationCurr.Empty();
	OuthitActorCurr.Empty();

	/////
	FVector ForwardVec = ((GetActorForwardVector() * 2500.f) + Start);
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start + GetActorForwardVector() * 50, ForwardVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal")) 
		{
			OuthitActorCurr.Add(0);
			detectGoal.Add(0);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector LeftVec = GetActorForwardVector().RotateAngleAxis(-90.0f, FVector(0, 0, 1)) * 2500.f + Start;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, LeftVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			detectGoal.Add(270);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector LeftMidVec = GetActorForwardVector().RotateAngleAxis(-45.0f, FVector(0, 0, 1)) * 2500.f + Start;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, LeftMidVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			detectGoal.Add(315);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector RightVec = GetActorForwardVector().RotateAngleAxis(90.0f, FVector(0, 0, 1)) * 2500.f + Start;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, RightVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			detectGoal.Add(90);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector RightMidVec = GetActorForwardVector().RotateAngleAxis(45.0f, FVector(0, 0, 1)) * 2500.f + Start;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, RightMidVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			detectGoal.Add(45);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector BackwardVec = GetActorForwardVector().RotateAngleAxis(180.0f, FVector(0, 0, 1)) * 2500.f + Start;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start - GetActorForwardVector() * 50, BackwardVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			detectGoal.Add(180);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector BackwardLeftVec = GetActorForwardVector().RotateAngleAxis(225.0f, FVector(0, 0, 1)) * 2500.f + Start;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, BackwardLeftVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			detectGoal.Add(225);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector BackwardRightVec = GetActorForwardVector().RotateAngleAxis(135.0f, FVector(0, 0, 1)) * 2500.f + Start;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, BackwardRightVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			detectGoal.Add(135);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}


	//////
	FVector UpStart = Start + FVector(0, 0, jumpHeight);
	FVector UpForwardVec = ((GetActorForwardVector() * 2500.f) + UpStart);
	if (GetWorld()->LineTraceSingleByChannel(OutHit, UpStart, UpForwardVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			if (detectGoal.Find(0) == INDEX_NONE) detectGoal.Add(0);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector UpLeftVec = GetActorForwardVector().RotateAngleAxis(-90.0f, FVector(0, 0, 1)) * 2500.f + UpStart;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, UpStart, UpLeftVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			if (detectGoal.Find(270) == INDEX_NONE) detectGoal.Add(270);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector UpLeftMidVec = GetActorForwardVector().RotateAngleAxis(-45.0f, FVector(0, 0, 1)) * 2500.f + UpStart;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, UpStart, UpLeftMidVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			if (detectGoal.Find(315) == INDEX_NONE) detectGoal.Add(315);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector UpRightVec = GetActorForwardVector().RotateAngleAxis(90.0f, FVector(0, 0, 1)) * 2500.f + UpStart;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, UpStart, UpRightVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			if (detectGoal.Find(90) == INDEX_NONE) detectGoal.Add(90);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector UpRightMidVec = GetActorForwardVector().RotateAngleAxis(45.0f, FVector(0, 0, 1)) * 2500.f + UpStart;
	if (GetWorld()->LineTraceSingleByChannel(OutHit,UpStart, UpRightMidVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			if (detectGoal.Find(45) == INDEX_NONE) detectGoal.Add(45);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector UpBackwardVec = GetActorForwardVector().RotateAngleAxis(180.0f, FVector(0, 0, 1)) * 2500.f + UpStart;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, UpStart - GetActorForwardVector() * 50, UpBackwardVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			if (detectGoal.Find(180) == INDEX_NONE) detectGoal.Add(180);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector UpBackwardLeftVec = GetActorForwardVector().RotateAngleAxis(225.0f, FVector(0, 0, 1)) * 2500.f + UpStart;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, UpStart, UpBackwardLeftVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			if (detectGoal.Find(225) == INDEX_NONE) detectGoal.Add(225);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	FVector UpBackwardRightVec = GetActorForwardVector().RotateAngleAxis(135.0f, FVector(0, 0, 1)) * 2500.f + UpStart;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, UpStart, UpBackwardRightVec, ECC_EngineTraceChannel4, CollisionParams)) {
		observationCurr.Add(std::min(25.0f, OutHit.Distance / 100));
		if (OutHit.GetActor()->ActorHasTag("wall")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("obstacle")) OuthitActorCurr.Add(1);
		else if (OutHit.GetActor()->ActorHasTag("stair")) OuthitActorCurr.Add(2);
		else if (OutHit.GetActor()->ActorHasTag("goal"))
		{
			OuthitActorCurr.Add(0);
			if (detectGoal.Find(135) == INDEX_NONE) detectGoal.Add(135);
		}
		else OuthitActorCurr.Add(0);
	}
	else {
		observationCurr.Add(25.0f);
		OuthitActorCurr.Add(0);
	}

	//////

	FVector DownVec2 = FVector(0, 0, -1) * 700.f + Start;
	if (!isSendObservation) {
		if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, DownVec2, ECC_EngineTraceChannel4, CollisionParams)) {
			if (OutHit.GetComponent()->GetName() == "Stair1") {
				agentGround = 1;
			}
			else if (OutHit.GetComponent()->GetName() == "Stair2") {
				agentGround = 2;
			}
			else if (OutHit.GetComponent()->ComponentHasTag("stair3")) {
				agentGround = 3;
			}
			else agentGround = 0;
		}
		else {
			agentGround = 0;
		}
	}
	goalDistanceCurr = std::min(25.0f, GetDistanceTo(agentGoal) / 100);
	
	FVector forwardVector = FVector(GetActorForwardVector().X,
		GetActorForwardVector().Y, 0);
	FVector goalVector = FVector(
		agentGoal->GetActorLocation().X - GetActorLocation().X,
		agentGoal->GetActorLocation().Y - GetActorLocation().Y, 0);
	forwardVector.Normalize();
	goalVector.Normalize();
	goalAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(forwardVector, goalVector)));
	
	FVector cross = FVector::CrossProduct(forwardVector, goalVector);
	if (cross.Z < 0) goalAngle *= -1;

	//UE_LOG(LogTemp, Warning, TEXT("%d %d %d %d %d %d %d %d"), OutHitName[0], OutHitName[1], OutHitName[2], OutHitName[3], OutHitName[4], OutHitName[5], OutHitName[6], OutHitName[7]);

	//// 디버그용
	//DrawDebugLine(GetWorld(), Start, ForwardVec, FColor::Red, false, 1.0f);
	//DrawDebugLine(GetWorld(), Start, LeftVec, FColor::Red, false, 1.0f);
	//DrawDebugLine(GetWorld(), Start, RightVec, FColor::Red, false, 1.0f);
	//DrawDebugLine(GetWorld(), Start, BackwardVec, FColor::Red, false, 1.0f);

	//DrawDebugLine(GetWorld(), UpStart, UpForwardVec, FColor::Red, false, 1.0f);
	//DrawDebugLine(GetWorld(), UpStart, UpLeftVec, FColor::Red, false, 1.0f);
	//DrawDebugLine(GetWorld(), UpStart, UpRightVec, FColor::Red, false, 1.0f);
	//DrawDebugLine(GetWorld(), UpStart, UpBackwardVec, FColor::Red, false, 1.0f);

	//DrawDebugLine(GetWorld(), Start, LeftMidVec, FColor::Blue, false, 1.0f);
	//DrawDebugLine(GetWorld(), Start, RightMidVec, FColor::Green, false, 1.0f);
	//DrawDebugLine(GetWorld(), Start, BackwardLeftVec, FColor::Blue, false, 1.0f);
	//DrawDebugLine(GetWorld(), Start, BackwardRightVec, FColor::Green, false, 1.0f);

	//DrawDebugLine(GetWorld(), UpStart, UpLeftMidVec, FColor::Blue, false, 1.0f);
	//DrawDebugLine(GetWorld(), UpStart, UpRightMidVec, FColor::Green, false, 1.0f);
	//DrawDebugLine(GetWorld(), UpStart, UpBackwardLeftVec, FColor::Blue, false, 1.0f);
	//DrawDebugLine(GetWorld(), UpStart, UpBackwardRightVec, FColor::Green, false, 1.0f);

	//DrawDebugLine(GetWorld(), Start, DownVec2, FColor::Cyan, false, 1.0f);

}

// count time
void APPOCharacter::AdvanceTimer() {
	--CountdownTime;
	if (CountdownTime < 1)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		isTimeOver = true;
	}
}

void APPOCharacter::ApplySpeed(float DeltaTime) {
	movementSpeed *= 0.9f;
	rotateSpeed *= 0.9f;
}

AAgentLevelManager* APPOCharacter::GetLevelManager()
{
	return levelManager;
}

APPOGoal* APPOCharacter::GetAgentGoal()
{
	return agentGoal;
}