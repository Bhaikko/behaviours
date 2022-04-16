#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AiBehaviors/AiBehaviorsCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CoverHandler.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AIBEHAVIORS_API UCoverHandler : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCoverHandler();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Handlers, meta = (AllowPrivateAccess = "true"))
	AAiBehaviorsCharacter* ownerRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Handlers, meta = (AllowPrivateAccess = "true"))
	UCharacterMovementComponent* movementComponentRef;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere, Category = Cover)
	float coverDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Cover)
	bool isInCover;

public:
	UFUNCTION()
	void TryCover(float forward, float right);
	
	UFUNCTION()
	void StartCover(FHitResult& hitResult);

	UFUNCTION()
	void StopCover();

	// Used to Align player with cover when moving to always stick to cover
	UFUNCTION()
	void CoverTrace();

private:
	bool WallTrace(float forward, float right, FHitResult& hitResult);

	void MaintainCover(FHitResult& hitResult);


};
