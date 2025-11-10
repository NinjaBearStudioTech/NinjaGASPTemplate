// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterOverlayBase.generated.h"

UENUM(BlueprintType, DisplayName = "Overlay Base")
enum class ECharacterOverlayBase : uint8
{
	GASP,
	ALS,
	Masculine,
	Feminine,
	None = 255		UMETA(Hidden)
};