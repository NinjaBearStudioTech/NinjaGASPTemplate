// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterTraversalAction.generated.h"

UENUM(BlueprintType, DisplayName = "Traversal Action")
enum class ECharacterTraversalAction : uint8
{
	None,

	/** Traverse over a thin object and end on the ground at a similar level (Low fence). */
	Hurdle,

	/** Traverse over a thin object and end in a falling state (Tall fence, or elevated obstacle with no floor on the other side). */
	Vault,

	/** Traverse up and onto an object without passing over it. */
	Mantle
	
};