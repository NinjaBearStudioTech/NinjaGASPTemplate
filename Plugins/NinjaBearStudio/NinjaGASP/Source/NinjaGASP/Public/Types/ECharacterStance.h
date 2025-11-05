// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterStance.generated.h"

UENUM(BlueprintType, DisplayName = "Character Stance")
enum class ECharacterStance : uint8
{
	Stand,
	Crouch,
};