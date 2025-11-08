// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterTraversalOutcome.generated.h"

UENUM(BlueprintType, DisplayName = "Traversal Outcome")
enum class ECharacterTraversalOutcome : uint8
{
	NotInitialized,
	FailedToInitialize,
	TraversalNotFound,
	NotEnoughRoomOnTop,
	MontageSelectionFailed,
	TraversalFound
};