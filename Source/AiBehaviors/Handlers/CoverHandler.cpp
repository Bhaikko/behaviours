#include "AiBehaviors/Handlers/CoverHandler.h"

#include "DrawDebugHelpers.h"

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
}

bool UCoverHandler::WallTrace(float forward, float right, FHitResult& hitResult)
{
	FVector direction = FVector(forward, right, 0.0f);

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
	movementComponentRef->SetPlaneConstraintNormal(hitresult.Normal);
}

void UCoverHandler::StopCover()
{
	isInCover = false;
	movementComponentRef->SetPlaneConstraintEnabled(false);
}

