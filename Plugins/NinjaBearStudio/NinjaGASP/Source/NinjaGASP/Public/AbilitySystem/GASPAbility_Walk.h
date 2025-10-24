// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASGameplayAbility.h"
#include "GASPAbility_Walk.generated.h"

/**
 * Integrates with the Advanced Movement Interface to set the movement mode to walk.
 *
 * This can also be granted by default, as a passive ability, so the character is set
 * to walk by default, inverting the default behavior to "walk/jog" instead of "jog/walk".
 *
 * If you do the "walk/jog", you might want to set the "jog" ability to enable walk when
 * it ends. This is necessary since the GASP's default state is actually jogging.
 */
UCLASS()
class NINJAGASP_API UGASPAbility_Walk : public UNinjaGASGameplayAbility
{
	
	GENERATED_BODY()

public:

	UGASPAbility_Walk();

	/**
	 * Checks if the owner is currently walking.
	 */
	UFUNCTION(BlueprintPure, Category = "GASP|Abilities|Walk")
	bool IsWalking() const;
	
protected:

	// -- Begin Gameplay Ability implementation
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// -- End Gameplay Ability implementation

};
