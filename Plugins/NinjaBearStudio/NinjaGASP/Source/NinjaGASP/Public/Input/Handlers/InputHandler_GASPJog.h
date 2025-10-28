// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Input/Handlers/InputHandler_AbilityTag.h"
#include "InputHandler_GASPJog.generated.h"

/**
 * Invokes the Jog Ability that should interact with the Advanced Movement interface.
 *
 * This handler is not really used by the default "GASP" implementation since jogging is
 * the default movement mode. But for something like a survival horror, were walking is
 * the default movement mode, having an input for jog is useful.
 */
UCLASS(DisplayName = "GASP: Jog")
class NINJAGASP_API UInputHandler_GASPJog : public UInputHandler_AbilityTag
{
	GENERATED_BODY()
};
