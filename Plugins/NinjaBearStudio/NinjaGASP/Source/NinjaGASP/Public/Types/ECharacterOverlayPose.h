// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterOverlayPose.generated.h"

UENUM(BlueprintType, DisplayName = "Overlay Pose")
enum class ECharacterOverlayPose : uint8
{
	Default,
	Rifle,
	Pistol1H		UMETA(DisplayName = "Pistol 1H"),
	Pistol2H		UMETA(DisplayName = "Pistol 2H"),
	Bow,
	Torch,
	Binoculars,
	Box,
	Barrel,
	Injured,
	HandsTied,
	None = 255		UMETA(Hidden)
};