// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Types/ECharacterGait.h"
#include "Types/ECharacterMovementMode.h"
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
	 * Provides the gait currently assigned to the character.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	ECharacterGait GetCharacterGait() const;
	virtual ECharacterGait GetCharacterGait_Implementation() const { return ECharacterGait::Run; }

	/**
	 * Provides a vector with all walk speeds for the character.
	 * X = Forward Speed; Y = Strafe Speed; Z = Backwards Speed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	FVector GetWalkSpeeds() const;
	virtual FVector GetWalkSpeeds_Implementation() const { return FVector(200.f, 180.f, 150.f); }

	/**
	 * Provides a vector with all run speeds for the character.
	 * X = Forward Speed; Y = Strafe Speed; Z = Backwards Speed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	FVector GetRunSpeeds() const;
	virtual FVector GetRunSpeeds_Implementation() const { return FVector(500.f, 350.f, 300.f); }

	/**
	 * Provides a vector with all sprint speeds for the character.
	 * X = Forward Speed; Y = Strafe Speed; Z = Backwards Speed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	FVector GetSprintSpeeds() const;
	virtual FVector GetSprintSpeeds_Implementation() const { return FVector(700.f, 700.f, 700.f); }	

	/**
	 * Provides a vector with all crouch speeds for the character.
	 * X = Forward Speed; Y = Strafe Speed; Z = Backwards Speed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	FVector GetCrouchSpeeds() const;
	virtual FVector GetCrouchSpeeds_Implementation() const { return FVector(225.f, 200.f, 180.f); }	

	/**
	 * A curve that maps rotations (x) to different speeds (y).
	 * Used to calculate max speed when the character is strafing.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	UCurveFloat* GetStrafeSpeedMapCurve() const;
	virtual UCurveFloat* GetStrafeSpeedMapCurve_Implementation() const { return nullptr; }
	
	/**
	 * Informs if the character is currently walking.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	bool IsWalking() const;
	virtual bool IsWalking_Implementation() const { return false; }

	/**
	 * Informs if the character is currently sprinting.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	bool IsSprinting() const;
	virtual bool IsSprinting_Implementation() const { return false; }

	/**
	 * Checks if the character is actively running.
	 *
	 * This means the intent is set and the character is actually moving. Note that
	 * the "run" intent is usually set when no other intent that affects it is set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	bool IsActivelyRunning() const;
	virtual bool IsActivelyRunning_Implementation() const { return false; }
	
	/**
	 * Checks if the character is actively sprinting.
	 * 
	 * This means the intent is set, but the character might be moving at an angle
	 * that does not support sprinting, or not even moving at all.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	bool IsActivelySprinting() const;
	virtual bool IsActivelySprinting_Implementation() const { return false; }
	
	/**
	 * Informs if the character is currently strafing.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	bool IsStrafing() const;
	virtual bool IsStrafing_Implementation() const { return false; }

	/**
	 * Informs if the character is currently aiming.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	bool IsAiming() const;
	virtual bool IsAiming_Implementation() const { return false; }

	/**
	 * Informs if the character is in ragdoll mode.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	bool InRagdoll() const;
	virtual bool InRagdoll_Implementation() const { return false; }

	/**
	 * Informs if the character has landed recently.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	bool HasJustLanded() const;
	virtual bool HasJustLanded_Implementation() const { return false; }
	
	/**
	 * Registers the walking intent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	void SetWalkingIntent(bool bWantsToWalk);
	virtual void SetWalkingIntent_Implementation(bool bWantsToWalk) { }

	/**
	 * Registers the sprinting intent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	void SetSprintingIntent(bool bWantsToSprint);
	virtual void SetSprintingIntent_Implementation(bool bWantsToSprint) { }

	/**
	 * Registers the strafing intent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	void SetStrafingIntent(bool bWantsToStrafe);
	virtual void SetStrafingIntent_Implementation(bool bWantsToStrafe) { }

	/**
	 * Registers the aiming intent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	void SetAimingIntent(bool bWantsToAim);
	virtual void SetAimingIntent_Implementation(bool bWantsToAim) { }

	/**
	 * Provides the last velocity when the character landed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Advanced Character Movement Interface")
	FVector GetLandVelocity() const;

};
