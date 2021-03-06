// Copyright Epic Games, Inc. All Rights Reserved.

#include "AiBehaviorsCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"

#include "AiBehaviors/Handlers/CoverHandler.h"

//////////////////////////////////////////////////////////////////////////
// AAiBehaviorsCharacter

AAiBehaviorsCharacter::AAiBehaviorsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CoverHandler = CreateDefaultSubobject<UCoverHandler>(TEXT("Cover Handler"));

	IkHipOffset = 0.0f;
	leftHandHitWall = false;
	rightHandHitWall = false;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AAiBehaviorsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AAiBehaviorsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AAiBehaviorsCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AAiBehaviorsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AAiBehaviorsCharacter::LookUpAtRate);

	// Cover Key Binding
	PlayerInputComponent->BindAction("Cover", IE_Pressed, this, &AAiBehaviorsCharacter::TryCover);
}

void AAiBehaviorsCharacter::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
	HandleIKForLegs(deltaSeconds);
	HandleIKForHands(deltaSeconds);
}

void AAiBehaviorsCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AAiBehaviorsCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AAiBehaviorsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AAiBehaviorsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AAiBehaviorsCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AAiBehaviorsCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AAiBehaviorsCharacter::TryCover()
{
	CoverHandler->TryCover(InputComponent->GetAxisValue("MoveForward"), InputComponent->GetAxisValue("MoveRight"));
}

bool AAiBehaviorsCharacter::IKFootTrace(FName socketName, float distance, FHitResult& hitResult)
{
	FVector socketLocation = GetMesh()->GetSocketLocation(socketName);
	FVector actorLocation = GetActorLocation();

	FVector lineTraceStart = FVector(socketLocation.X, socketLocation.Y, actorLocation.Z);
	FVector lineTraceEnd = FVector(socketLocation.X, socketLocation.Y, socketLocation.Z - distance);

	return GetWorld()->LineTraceSingleByChannel(hitResult, lineTraceStart, lineTraceEnd, ECollisionChannel::ECC_Visibility);
}

bool AAiBehaviorsCharacter::IKHandTrace(FName socketName, float distance, FHitResult& hitResult)
{
	FVector socketLocation = GetMesh()->GetSocketLocation(socketName);
	FVector traceEnd = socketLocation + GetActorForwardVector() * distance;

	return GetWorld()->LineTraceSingleByChannel(hitResult, socketLocation, traceEnd, ECollisionChannel::ECC_Visibility);

}

void AAiBehaviorsCharacter::HandleIKForLegs(float deltaSeconds)
{
	FHitResult leftFootIK, rightFootIK;

	bool leftHit = IKFootTrace(FName("foot_lSocket"), GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), leftFootIK);
	bool rightHit = IKFootTrace(FName("foot_rSocket"), GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), rightFootIK);

	float 	leftFootTraceOffset = 0.0f,
			rightFootTraceOffset = 0.0f;
		

	if (leftHit) {
		leftFootTraceOffset = (leftFootIK.Location - GetMesh()->GetComponentLocation()).Z - IkHipOffset;

		IkLeftFootOffset = FMath::FInterpTo(IkLeftFootOffset, leftFootTraceOffset, deltaSeconds, 20.0f);
		IkLeftFootSurfaceNormal = leftFootIK.Normal;

		float rollRot = FMath::RadiansToDegrees(FMath::Atan2(IkLeftFootSurfaceNormal.Y, IkLeftFootSurfaceNormal.Z));
		float pitchRot = FMath::RadiansToDegrees(FMath::Atan2(IkLeftFootSurfaceNormal.X, IkLeftFootSurfaceNormal.Z)) * -1.0f;

		FRotator newRotation = FRotator(pitchRot, 0.0f, rollRot);
		IkLeftFootRotation = FMath::RInterpTo(IkLeftFootRotation, newRotation, deltaSeconds, 100.0f);
	}

	
	if (rightHit) {
		// DrawDebugLine(GetWorld(), GetActorLocation(), leftFootIK.Location, FColor::Red, false, 0.5f);
		rightFootTraceOffset = (rightFootIK.Location - GetMesh()->GetComponentLocation()).Z - IkHipOffset;

		IkRightFootOffset = FMath::FInterpTo(IkRightFootOffset, rightFootTraceOffset, deltaSeconds, 20.0f);
		IkRightFootSurfaceNormal = rightFootIK.Normal;

		float rollRot = FMath::RadiansToDegrees(FMath::Atan2(IkRightFootSurfaceNormal.Y, IkRightFootSurfaceNormal.Z));
		float pitchRot = FMath::RadiansToDegrees(FMath::Atan2(IkRightFootSurfaceNormal.X, IkRightFootSurfaceNormal.Z)) * -1.0f;

		FRotator newRotation = FRotator(pitchRot, 0.0f, rollRot);
		IkRightFootRotation = FMath::RInterpTo(IkLeftFootRotation, newRotation, deltaSeconds, 100.0f);
	}
	
	float hipOffset = FMath::Abs((leftFootIK.Location - rightFootIK.Location).Z);
	hipOffset = hipOffset < 50.0f ? hipOffset * -0.5f : 0.0f;

	IkHipOffset = FMath::FInterpTo(IkHipOffset, hipOffset, deltaSeconds, 20.0f);
}

void AAiBehaviorsCharacter::HandleIKForHands(float deltaSeconds)
{
	FHitResult leftHandIK, rightHandIK;

	leftHandHitWall = IKHandTrace(FName("upperarm_lSocket"), 100.0f, leftHandIK);
	rightHandHitWall = IKHandTrace(FName("upperarm_rSocket"), 100.0f, rightHandIK);

	IKLeftHandLocation = leftHandIK.Location;
	IKRightHandLocation = rightHandIK.Location;
}
