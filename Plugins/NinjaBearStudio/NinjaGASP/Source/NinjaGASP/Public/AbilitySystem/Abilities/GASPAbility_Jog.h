// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASGameplayAbility.h"
#include "AbilitySystem/NinjaGASPBaseLocomotionCostAbility.h"
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
 *
 * This ability supports cost (e.g. "stamina consumption").
 */
UCLASS()
class NINJAGASP_API UGASPAbility_Jog : public UNinjaGASPBaseLocomotionCostAbility
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

	/** Determines if this ability should revert to "walk" when it ends. */
	UPROPERTY(EditDefaultsOnly, Category = "Jog")
	bool bActivateWalkOnEnd;
	
	/** Tags used to activate the walk ability, when jog ends. */
	UPROPERTY(EditDefaultsOnly, Category = "Jog", meta = (EditCondition = "bActivateWalkOnEnd"))
	FGameplayTagContainer WalkAbilityActivationTags;

	virtual bool ActivateLocomotionMode_Implementation() override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
};
