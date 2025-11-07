// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterMovementDirectionBias.generated.h"

UENUM(BlueprintType, DisplayName = "Movement Direction Bias")
enum class ECharacterMovementDirectionBias : uint8
{
	LeftFootForward,
	RightFootForward,
};