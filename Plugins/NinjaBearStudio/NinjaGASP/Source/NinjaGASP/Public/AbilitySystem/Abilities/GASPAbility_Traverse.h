// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASGameplayAbility.h"
#include "Types/FCharacterMovementTraversalCheckResult.h"
#include "GASPAbility_Traverse.generated.h"

class UChooserTable;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitTraversableObstacle;

/**
 * Handles the traversal check and action. If that fails, activates a fallback.
 */
UCLASS()
class NINJAGASP_API UGASPAbility_Traverse : public UNinjaGASGameplayAbility
{
	
	GENERATED_BODY()

public:

	UGASPAbility_Traverse();

protected:

	/** Trace type for the traversal object. It's recommended to create a dedicated channel for this! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
	TEnumAsByte<ECollisionChannel> TraversalChannel;
	
	/** Chooser table including all traversal animations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UChooserTable> TraversalAnimations;

	/** Ability to activate, if no traversal is possible. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
	FGameplayTagContainer FallbackAbilityTags;
	
	/** Ability Task used to detected traversable objects. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
	TSubclassOf<UAbilityTask_WaitTraversableObstacle> TraversableTaskClass;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	/**
	 * If set, overrides the Animation Instance used by the Ability Task.
	 * Returning a null value will have no effect to the Ability Task.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Traversal Ability")
	UAnimInstance* GetAnimInstance() const;
	virtual UAnimInstance* GetAnimInstance_Implementation() const { return nullptr; }

	/**
	 * Executes a fallback from the invalid traversal attempt.
	 *
	 * By default, tries to activate an ability defined by the provided fallback tags.
	 * You may want to override this to do something more direct, such as "character jump".
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Traversal Ability")
	void FallbackFromInvalidTraversal();
	
	/** Invoked when the traversable obstacle search completes. */
	UFUNCTION()
	void HandleTraversableObstacleSearchFinished(const FCharacterMovementTraversalCheckResult& Result);

	/** Invoked when the montage has successfully ended. */
	UFUNCTION()
	void HandleMontageEnded();

	/** Invoked when the montage has been interrupted/cancelled. */
	UFUNCTION()
	void HandleMontageCancelled();
	
private:

	bool bStartedTraversalAction;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitTraversableObstacle> TraversableTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
	
};
