// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterMovementMode.generated.h"

UENUM(BlueprintType, DisplayName = "Movement Mode")
enum class ECharacterMovementMode : uint8
{
	OnGround,
	InAir,
	Flying,
	Swimming
};