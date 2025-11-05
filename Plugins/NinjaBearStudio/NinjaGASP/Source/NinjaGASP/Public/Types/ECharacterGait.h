// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterGait.generated.h"

UENUM(BlueprintType, DisplayName = "Character Gait")
enum class ECharacterGait : uint8
{
	Walk,
	Run,
	Sprint
};