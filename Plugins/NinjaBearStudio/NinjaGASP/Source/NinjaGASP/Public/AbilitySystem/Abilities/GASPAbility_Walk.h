// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASGameplayAbility.h"
#include "AbilitySystem/NinjaGASPBaseLocomotionAbility.h"
#include "GASPAbility_Walk.generated.h"

/**
 * Integrates with the Advanced Movement Interface to set the walk intent.
 *
 * This can also be granted by default, as a passive ability, so the character is set
 * to walk by default, inverting the default behavior to "walk/jog" instead of "jog/walk".
 * You can grant passive abilities with the tag "Ability.Passive".
 *
 * You can also control the activation behavior between "toggled" and "momentary", using
 * Ninja Input for that ("input.ability.toggled" and "input.ability.momentary").
 *
 * If you do the "walk/jog", you might want to set the "jog" ability to enable walk when
 * it ends. This is necessary since the GASP's default state is actually jogging.
 */
UCLASS()
class NINJAGASP_API UGASPAbility_Walk : public UNinjaGASPBaseLocomotionAbility
{
	
	GENERATED_BODY()

public:

	UGASPAbility_Walk();

	/** Checks if the owner is currently walking. */
	bool IsWalking() const;
	
protected:

	virtual bool ActivateLocomotionMode_Implementation() override;
	virtual void DeactivateLocomotionMode_Implementation() override;

};
