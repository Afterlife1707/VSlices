// Copyright Epic Games, Inc. All Rights Reserved.

#include "VSlicesGameMode.h"
#include "VSlicesCharacter.h"
#include "UObject/ConstructorHelpers.h"

AVSlicesGameMode::AVSlicesGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
