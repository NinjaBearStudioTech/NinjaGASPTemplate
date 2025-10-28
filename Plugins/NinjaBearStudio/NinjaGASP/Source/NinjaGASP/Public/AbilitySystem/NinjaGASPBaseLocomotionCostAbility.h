// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASGameplayAbility.h"
#include "NinjaGASPBaseLocomotionCostAbility.generated.h"

class UAbilityTask_NetworkSyncPoint;

/**
 * Allows locomotion modes, such as "Sprinting" to have a cost, such as "stamina".
 *
 * If the cost is set, it will be applied constantly, while the ability is active.
 * Otherwise, if no cost is set, the ability basically executes as normal locomotion.
 */
UCLASS(Abstract)
class NINJAGASP_API UNinjaGASPBaseLocomotionCostAbility : public UNinjaGASGameplayAbility
{
	
	GENERATED_BODY()

public:

	UNinjaGASPBaseLocomotionCostAbility();

protected:

	/** How often the cost is applied. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recurring Cost")
	float CostApplicationInterval;
	
	/** Maximum time the server will wait for the net sync. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recurring Cost")
	float MaxServerWaitTime;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
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
	 * Confirms we can activate the locomotion mode for the fist time.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Locomotion")
	bool CanActivateLocomotionMode() const;

	/**
	 * Confirms we can keep the locomotion active after a cost reapplication.
	 * 
	 * By default, follows the same logic as "CanActivateLocomotionMode", so you only
	 * need to implement this if these checks have different conditions.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Locomotion")
	bool CanKeepLocomotionModeActive() const;
	
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
	
private:

	/**
	 * Tracks changes done in the locomotion mode.
	 * This also affects the cooldown application.
	 */
	bool bChangedLocomotionMode;

	/** Timer for the next synchronization. */
	FTimerHandle NetSyncScheduledTimer;
	
	/** Net sync used for the cost prediction window. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_NetworkSyncPoint> NetSync;
	
};
