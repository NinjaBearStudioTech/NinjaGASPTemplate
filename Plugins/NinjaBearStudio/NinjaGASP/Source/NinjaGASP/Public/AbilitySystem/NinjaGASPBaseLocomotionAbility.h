// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASGameplayAbility.h"
#include "NinjaGASPBaseLocomotionAbility.generated.h"

/**
 * Base gameplay ability for the GASP locomotion modes.
 */
UCLASS(Abstract)
class NINJAGASP_API UNinjaGASPBaseLocomotionAbility : public UNinjaGASGameplayAbility
{
	
	GENERATED_BODY()

public:

	UNinjaGASPBaseLocomotionAbility();

	/** Informs if this ability, if active, has changed the locomotion mode. */
	bool HasChangedLocomotionMode() const { return bChangedLocomotionMode; }
		
protected:

	/**
	 * Gameplay Effect applied while the locomotion ability is active.
	 * The effect is applied using the ability level.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	TSubclassOf<UGameplayEffect> LocomotionEffectClass;
	
	/** Tracks changes done in the locomotion mode. May affect the cooldown application. */
	bool bChangedLocomotionMode;
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/**
	 * Confirms we can activate the locomotion mode for the fist time.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Locomotion")
	bool CanActivateLocomotionMode() const;

	/**
	 * Activates the locomotion mode relevant to this ability.
	 *
	 * This is invoked when the cost is applied for the first time, as the ability activates,
	 * or right away when the ability activates and there is no cost Gameplay Effect assigned.
	 *
	 * @return
	 *		Boolean informing if the related locomotion mode was activated.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Locomotion")
	bool ActivateLocomotionMode();

	/**
	 * Deactivates the locomotion mode relevant to this ability.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Locomotion")
	void DeactivateLocomotionMode();

	/** Applies the locomotion effect set in the ability, if any. */
	virtual void ApplyLocomotionEffect();

	/** Removes the locomotion effect set in the ability, if any. */
	virtual void RemoveLocomotionEffect();

private:

	/** Handle representing the active locomotion gameplay effect. */
	FActiveGameplayEffectHandle LocomotionGameplayEffectHandle;	
	
};
