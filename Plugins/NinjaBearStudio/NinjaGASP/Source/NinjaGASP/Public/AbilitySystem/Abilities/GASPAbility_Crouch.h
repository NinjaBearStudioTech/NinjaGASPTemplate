// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASPBaseLocomotionAbility.h"
#include "GASPAbility_Crouch.generated.h"

/**
 * Integrates with the base character to modify the crouch state.
 */
UCLASS()
class NINJAGASP_API UGASPAbility_Crouch : public UNinjaGASPBaseLocomotionAbility
{
	
	GENERATED_BODY()

public:

	UGASPAbility_Crouch();
	
protected:

	virtual bool ActivateLocomotionMode_Implementation() override;
	virtual void DeactivateLocomotionMode_Implementation() override;
	
};
