// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "gravitygunGameMode.h"
#include "gravitygunHUD.h"
#include "gravitygunCharacter.h"
#include "UObject/ConstructorHelpers.h"

AgravitygunGameMode::AgravitygunGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AgravitygunHUD::StaticClass();
}
