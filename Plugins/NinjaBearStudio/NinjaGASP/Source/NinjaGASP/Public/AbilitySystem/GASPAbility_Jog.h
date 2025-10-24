// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASGameplayAbility.h"
#include "GASPAbility_Jog.generated.h"

/**
 * Integrates with the Advanced Movement Interface to set the movement mode to jog.
 *
 * This is actually something that you don't need by default, since "jogging" is the
 * default state for the GASP character. However, if you are making a game where "walking"
 * is the default state, then your Walk Ability will be activated by default and this
 * would be activated via user Input (classic setup for a survival horror type of game).
 *
 * For that scenario, this ability will interrupt an active walk ability, allowing the
 * GASP character to reset to the jogging state. When this ability ends, it will attempt
 * to re-activate the Walk ability.
 */
UCLASS()
class NINJAGASP_API UGASPAbility_Jog : public UNinjaGASGameplayAbility
{
	
	GENERATED_BODY()

public:

	UGASPAbility_Jog();
	
	/**
	 * Checks if the owner is currently jogging.
	 * The character is jogging when not walking nor sprinting.
	 */
	UFUNCTION(BlueprintPure, Category = "GASP|Abilities|Jog")
	bool IsJogging() const;	

protected:

	/** Tags used to activate the walk ability, when jog ends. */
	UPROPERTY(EditDefaultsOnly, Category = "Jog")
	FGameplayTagContainer WalkAbilityActivationTags;
	
	// -- Begin Gameplay Ability implementation
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// -- End Gameplay Ability implementation

	/**
	 * Tries to activate the Walk Ability, using the pre-defined activation tags.
	 * Should be invoked **after** this ability ends, to avoid tag clashes!
	 */
	void TryActivateWalkAbility() const;
};
