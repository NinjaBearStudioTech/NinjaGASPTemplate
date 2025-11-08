// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASPBaseLocomotionAbility.h"
#include "GASPAbility_Strafe.generated.h"

/**
 * Integrates with the Advanced Movement Interface to set the strafe intent.
 */
UCLASS()
class NINJAGASP_API UGASPAbility_Strafe : public UNinjaGASPBaseLocomotionAbility
{
	
	GENERATED_BODY()

public:

	UGASPAbility_Strafe();
	
protected:

	virtual bool ActivateLocomotionMode_Implementation() override;
	virtual void DeactivateLocomotionMode_Implementation() override;
	
};
