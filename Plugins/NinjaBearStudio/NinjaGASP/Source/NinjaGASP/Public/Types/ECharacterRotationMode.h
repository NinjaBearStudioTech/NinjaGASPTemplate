// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterRotationMode.generated.h"

UENUM(BlueprintType, DisplayName = "Rotation Mode")
enum class ECharacterRotationMode : uint8
{
	/** The character rotates toward the direction of movement (traditional 3rd person action adventure style). */
	OrientToMovement,

	/** The character rotates toward the camera direction (traditional 3rd person shooter style). */
	Strafe,
};