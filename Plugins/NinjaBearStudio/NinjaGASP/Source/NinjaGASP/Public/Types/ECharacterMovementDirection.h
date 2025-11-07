// Ninja Bear Studio Inc., all rights reserved.
// ReSharper disable CppUE4CodingStandardNamingViolationWarning
#pragma once

#include "CoreMinimal.h"
#include "ECharacterMovementDirection.generated.h"

UENUM(BlueprintType, DisplayName = "Movement Direction")
enum class ECharacterMovementDirection : uint8
{
	F, B, LL, LR, RL, RR
};