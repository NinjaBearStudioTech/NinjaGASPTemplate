// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASPBaseLocomotionAbility.h"
#include "GASPAbility_Aim.generated.h"

/**
 * Integrates with the Advanced Movement Interface to set the aim intent.
 */
UCLASS()
class NINJAGASP_API UGASPAbility_Aim : public UNinjaGASPBaseLocomotionAbility
{
	
	GENERATED_BODY()

public:

	UGASPAbility_Aim();
	
protected:

	virtual bool ActivateLocomotionMode_Implementation() override;
	virtual void DeactivateLocomotionMode_Implementation() override;
	
};
