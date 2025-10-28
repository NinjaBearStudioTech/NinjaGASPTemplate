// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Input/Handlers/InputHandler_AbilityTag.h"
#include "InputHandler_GASPJump.generated.h"

/**
 * Triggers the ability that can perform traversal movement or jump as a fallback. 
 */
UCLASS(DisplayName = "GASP: Jump / Vault")
class NINJAGASP_API UInputHandler_GASPJump : public UInputHandler_AbilityTag
{
	GENERATED_BODY()
};
