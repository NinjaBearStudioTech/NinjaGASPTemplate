// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AdvancedCharacterMovementInterface.generated.h"

UINTERFACE(MinimalAPI)
class UAdvancedCharacterMovementInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Handles advanced movement operations such as walk lock, sprinting and strafing.
 * 
 * These are not handled by the engine, and were introduced by GASP, so we want to create an
 * API for these, without relying on the character class itself, since we might have a few.
 */
class NINJAGASP_API IAdvancedCharacterMovementInterface
{
	
	GENERATED_BODY()

public:

	/**
	 * Informs if the character is currently walking.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Advanced Character Movement Interface")
	bool IsWalking() const;
	bool IsWalking_Implementation() const { return false; }

	/**
	 * Informs if the character is currently sprinting.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Advanced Character Movement Interface")
	bool IsSprinting() const;
	bool IsSprinting_Implementation() const { return false; }

	/**
	 * Informs if the character is currently strafing.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Advanced Character Movement Interface")
	bool IsStrafing() const;
	bool IsStrafing_Implementation() const { return false; }
	
	/**
	 * Registers the walking intent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Advanced Character Movement Interface")
	void SetWalkingIntent(bool bWantsToWalk);
	void SetWalkingIntent_Implementation(bool bWantsToWalk) const { }

	/**
	 * Registers the sprinting intent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Advanced Character Movement Interface")
	void SetSprintingIntent(bool bWantsToSprint);
	void SetSprintingIntent_Implementation(bool bWantsToSprint) const { }

	/**
	 * Registers the strafing intent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Advanced Character Movement Interface")
	void SetStrafingIntent(bool bWantsToStrafe);
	void SetStrafingIntent_Implementation(bool bWantsToStrafe) const { }
	
};

