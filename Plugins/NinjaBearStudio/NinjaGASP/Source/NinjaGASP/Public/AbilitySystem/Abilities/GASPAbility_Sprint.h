// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASPBaseLocomotionCostAbility.h"
#include "GASPAbility_Sprint.generated.h"

/**
 * Integrates with the Advanced Movement Interface to set the movement mode to sprint.
 * This ability supports cost (e.g. "stamina consumption").
 */
UCLASS()
class NINJAGASP_API UGASPAbility_Sprint : public UNinjaGASPBaseLocomotionCostAbility
{
	
	GENERATED_BODY()

public:

	UGASPAbility_Sprint();
	
	/**
	 * Checks if the owner is currently sprinting.
	 * The character is sprinting when the backend state is set.
	 */
	UFUNCTION(BlueprintPure, Category = "GASP|Abilities|Sprint")
	bool IsSprinting() const;	

protected:

	virtual bool ActivateLocomotionMode_Implementation() override;
	virtual void DeactivateLocomotionMode_Implementation() override;
	
};
