// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "EPlayerCameraMode.generated.h"

UENUM(BlueprintType)
enum class EPlayerCameraMode : uint8
{
	Balanced,
	Far,
	Close,
	FirstPerson
};