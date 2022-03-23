// Copyright Epic Games, Inc. All Rights Reserved.

#include "AiBehaviorsGameMode.h"
#include "AiBehaviorsCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAiBehaviorsGameMode::AAiBehaviorsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
