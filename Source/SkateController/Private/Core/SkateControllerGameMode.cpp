// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/SkateControllerGameMode.h"
#include "Core/SkateControllerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASkateControllerGameMode::ASkateControllerGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("Game/Blueprints/Core/BP_SkaterCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
