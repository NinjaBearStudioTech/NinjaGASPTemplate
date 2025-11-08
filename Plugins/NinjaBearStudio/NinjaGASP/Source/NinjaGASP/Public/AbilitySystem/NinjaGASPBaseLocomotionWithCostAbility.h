// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaGASPBaseLocomotionAbility.h"
#include "AbilitySystem/NinjaGASGameplayAbility.h"
#include "NinjaGASPBaseLocomotionWithCostAbility.generated.h"

class UAbilityTask_NetworkSyncPoint;

/**
 * Allows locomotion modes, such as "Sprinting" to have a cost, such as "stamina".
 *
 * If the cost is set, it will be applied constantly, while the ability is active.
 * Otherwise, if no cost is set, the ability basically executes as normal locomotion.
 */
UCLASS(Abstract)
class NINJAGASP_API UNinjaGASPBaseLocomotionWithCostAbility : public UNinjaGASPBaseLocomotionAbility
{
	
	GENERATED_BODY()

public:

	UNinjaGASPBaseLocomotionWithCostAbility();

protected:

	/** How often the cost is applied. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recurring Cost")
	float CostApplicationInterval;
	
	/** Maximum time the server will wait for the net sync. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recurring Cost")
	float MaxServerWaitTime;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/**
	 * Schedules the net sync for the net application interval.
	 */
	void ScheduleNetSync();

	/**
	 * Initializes the Net Sync task, considering the provided cooldown.
	 * It will also detach and interrupt from an ongoing instance.
	 */
	UFUNCTION()
	void InitializeNetSync();

	/**
	 * Moment where we have a prediction key and can apply the cost again.
	 * If the cost is valid, books another net sync. Otherwise, ends the ability.
	 */
	UFUNCTION()
	void OnServerSynchronized();

	/**
	 * Checks if the cost should be applied on a re-sync.
	 *
	 * This is important for things like sprinting, where the **intent** might be active
	 * (and thus the ability), but the character is not moving or the angle is not valid.
	 *
	 * This won't stop the ability, but won't apply the cost.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Locomotion")
	bool ShouldApplyCost() const;
	
	/**
	 * Confirms we can keep the locomotion active after a cost reapplication.
	 * 
	 * By default, follows the same logic as "CanActivateLocomotionMode", so you only
	 * need to implement this if these checks have different conditions.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Locomotion")
	bool CanKeepLocomotionModeActive() const;
	
private:

	/** Timer for the next synchronization. */
	FTimerHandle NetSyncScheduledTimer;
	
	/** Net sync used for the cost prediction window. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_NetworkSyncPoint> NetSync;
	
};
