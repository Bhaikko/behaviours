// Fill out your copyright notice in the Description page of Project Settings.


#include "AiBehaviors/Components/CoverHandler.h"

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

bool UCoverHandler::WallTrace(FHitResult& hitResult)
{
	FVector direction = FVector(ownerRef->GetInputAxisKeyValue("MoveForward"), ownerRef->GetInputAxisKeyValue("MoveRight"), 0.0f);

	DrawDebugLine(GetWorld(), ownerRef->GetActorLocation(), ownerRef->GetActorLocation() + direction * 100.0f, FColor::Red);

	return true;
}

void UCoverHandler::TryCover()
{
	
	FHitResult hitResult;
	bool foundCover = WallTrace(hitResult);
}

void UCoverHandler::StopCover()
{

}

