#include "AiBehaviors/Handlers/CoverHandler.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

// Sets default values for this component's properties
UCoverHandler::UCoverHandler()
{
	PrimaryComponentTick.bCanEverTick = true;

	isInCover = false;
}

void UCoverHandler::BeginPlay()
{
	Super::BeginPlay();

	ownerRef = Cast<AAiBehaviorsCharacter>(GetOwner());
	movementComponentRef = Cast<UCharacterMovementComponent>(ownerRef->GetComponentByClass(UCharacterMovementComponent::StaticClass()));
	
}

void UCoverHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (isInCover) {
		CoverTrace();
	}
}

bool UCoverHandler::WallTrace(float forward, float right, FHitResult& hitResult)
{
	FVector direction = ownerRef->GetActorForwardVector() * forward;
	direction.Normalize();

	// DrawDebugLine(GetWorld(), ownerRef->GetActorLocation(), ownerRef->GetActorLocation() + direction * coverDistance, FColor::Red, true);

	if (direction == FVector::ZeroVector) {
		direction = ownerRef->GetActorForwardVector();
	}

	return GetWorld()->LineTraceSingleByChannel(
		hitResult, 
		ownerRef->GetActorLocation(),
		ownerRef->GetActorLocation() + direction * coverDistance,
		ECollisionChannel::ECC_GameTraceChannel1
	);
}

void UCoverHandler::CoverTrace()
{
	FHitResult hitResult;
	float hitDistance = 200.0f;

	// Right hit Test
	FVector rightStart = ownerRef->GetMesh()->GetSocketLocation(FName("upperarm_r_CoverTrace"));
	FVector rightEnd = rightStart - movementComponentRef->GetPlaneConstraintNormal() * hitDistance;

	// DrawDebugLine(GetWorld(), rightStart, rightEnd, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), rightStart, 50.0f, 24, FColor::Red);

	bool rightHit = GetWorld()->LineTraceSingleByChannel(
		hitResult,
		rightStart,
		rightEnd,
		ECollisionChannel::ECC_GameTraceChannel1
	);

	// Left hit Test
	FVector leftStart = ownerRef->GetMesh()->GetSocketLocation(FName("upperarm_l_CoverTrace"));
	FVector leftEnd = leftStart - movementComponentRef->GetPlaneConstraintNormal() * hitDistance;

	// DrawDebugLine(GetWorld(), leftStart, leftEnd, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), rightStart, 50.0f, 24, FColor::Red);

	bool leftHit = GetWorld()->LineTraceSingleByChannel(
		hitResult,
		leftStart,
		leftEnd,
		ECollisionChannel::ECC_GameTraceChannel1
	);

	if (rightHit && leftHit) {
		float x = ownerRef->InputComponent->GetAxisValue("MoveRight");

		if (x != 0) {
			FVector start = ownerRef->GetActorLocation();
			FVector end = start + movementComponentRef->GetPlaneConstraintNormal() * -200.0f;

			if (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_GameTraceChannel1)) {
				MaintainCover(hitResult);
			}
		}
	}

}

void UCoverHandler::TryCover(float forward, float right)
{
	if (isInCover) {
		StopCover();
	}

	FHitResult hitResult;
	bool foundCover = WallTrace(forward, right, hitResult);

	if (foundCover) {
		StartCover(hitResult);
	}
}

void UCoverHandler::StartCover(FHitResult& hitresult)
{
	isInCover = true;
	movementComponentRef->SetPlaneConstraintEnabled(true);
	
	MaintainCover(hitresult);
}

void UCoverHandler::MaintainCover(FHitResult& hitresult)
{
	movementComponentRef->SetPlaneConstraintNormal(hitresult.Normal);
	
	movementComponentRef->bOrientRotationToMovement = false;

	FVector coverLocation = FVector(hitresult.Location.X, hitresult.Location.Y, ownerRef->GetActorLocation().Z);
	coverLocation += movementComponentRef->GetPlaneConstraintNormal() * 35.0f;

	DrawDebugSphere(GetWorld(), hitresult.Location, 50.0f, 24, FColor::Red);

	ownerRef->SetActorLocation(coverLocation);
	ownerRef->SetActorRotation(UKismetMathLibrary::MakeRotFromX(hitresult.Normal));
}

void UCoverHandler::StopCover()
{
	isInCover = false;
	movementComponentRef->SetPlaneConstraintEnabled(false);
	movementComponentRef->bOrientRotationToMovement = true;
}

