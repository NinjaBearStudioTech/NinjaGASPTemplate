// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NinjaGASPCharacter.h"
#include "NinjaGASPAICharacter.generated.h"

/**
 * AI variation of the base character.
 */
UCLASS(Abstract)
class NINJAGASP_API ANinjaGASPAICharacter : public ANinjaGASPCharacter
{
	
	GENERATED_BODY()

public:

	ANinjaGASPAICharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
};
