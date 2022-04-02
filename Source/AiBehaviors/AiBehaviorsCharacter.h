// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AiBehaviorsCharacter.generated.h"

UCLASS(config=Game)
class AAiBehaviorsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AAiBehaviorsCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float deltaSeconds) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; } 

protected:
	bool IKFootTrace(FName socketName, float distance, FHitResult& hitResult);

	bool IKHandTrace(FName socketName, float distance, FHitResult& hitResult);

	void HandleIKForLegs(float deltaSeconds);
	void HandleIKForHands(float deltaSeconds);

public:
	// Legs IK
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IkHipOffset;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IkLeftFootOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IkRightFootOffset;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FVector IkRightFootSurfaceNormal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FVector IkLeftFootSurfaceNormal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FRotator IkLeftFootRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FRotator IkRightFootRotation;
	
	// Hands IK
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	bool leftHandHitWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	bool rightHandHitWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FVector IKLeftHandLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FVector IKRightHandLocation;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Handlers, meta = (AllowPrivateAccess = "true"))
	class UCoverHandler* coverHandler;

protected:
	void TryCover();

};

