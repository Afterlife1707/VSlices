// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/VSlicesGameMode.h"
#include "UObject/ConstructorHelpers.h"

AVSlicesGameMode::AVSlicesGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/BP_Character"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
