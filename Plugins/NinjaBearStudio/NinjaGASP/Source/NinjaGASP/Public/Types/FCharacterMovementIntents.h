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

	static FCharacterMovementIntents ForSprinting(const bool bWantsToSprint)
	{
		FCharacterMovementIntents Intents;
		Intents.bWantsToSprint = bWantsToSprint;
		return Intents;
	}
	
	static FCharacterMovementIntents ForWalking(const bool bWantsToWalk)
	{
		FCharacterMovementIntents Intents;
		Intents.bWantsToWalk = bWantsToWalk;
		return Intents;
	}

	static FCharacterMovementIntents ForStrafing(const bool bWantsToStrafe)
	{
		FCharacterMovementIntents Intents;
		Intents.bWantsToStrafe = bWantsToStrafe;
		return Intents;
	}

	static FCharacterMovementIntents ForAiming(const bool bWantsToAim)
	{
		FCharacterMovementIntents Intents;
		Intents.bWantsToAim = bWantsToAim;
		return Intents;
	}
};