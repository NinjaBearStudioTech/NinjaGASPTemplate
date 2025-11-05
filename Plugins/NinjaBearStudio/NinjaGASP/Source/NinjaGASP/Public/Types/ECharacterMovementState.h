// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterMovementState.generated.h"

UENUM(BlueprintType, DisplayName = "Movement State")
enum class ECharacterMovementState : uint8
{
	Idle,
	Moving,
};