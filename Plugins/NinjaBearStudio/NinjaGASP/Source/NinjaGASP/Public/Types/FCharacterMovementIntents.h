// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FCharacterMovementIntents.generated.h"

/**
 * Possible intents for a character.
 */
USTRUCT(BlueprintType)
struct NINJAGASP_API FCharacterMovementIntents
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Intent")
	uint8 bWantsToSprint:1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Intent")
	uint8 bWantsToWalk:1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Intent")
	uint8 bWantsToStrafe:1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Intent")
	uint8 bWantsToAim:1 = false;

};