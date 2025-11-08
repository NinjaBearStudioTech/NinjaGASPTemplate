// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Types/FCharacterMovementTraversalCheckInputs.h"
#include "Types/FCharacterMovementTraversalCheckResult.h"
#include "AbilityTask_WaitTraversableObstacle.generated.h"

class UChooserTable;

/**
 * Tries to find a traversable object and broadcasts the result. 
 */
UCLASS()
class NINJAGASP_API UAbilityTask_WaitTraversableObstacle : public UAbilityTask
{

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTraversableObstacleFoundMCDelegate, FCharacterMovementTraversalCheckResult, Result);
	
	GENERATED_BODY()

public:

	/** A traversable obstacle has been found. */
	UPROPERTY(BlueprintAssignable)	
	FTraversableObstacleFoundMCDelegate TraversableObstacleSearchFinished;

	/**
	 * Tries to find a traversable object and broadcasts results.
	 */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability|Tasks")
	static UAbilityTask_WaitTraversableObstacle* CreateTask(UGameplayAbility* OwningAbility, ETraceTypeQuery TraversalTraceChannel, UChooserTable* TraversalChooserTable);

	// -- Begin Ability Task implementation
	virtual void Activate() override;
	// -- End Ability Task implementation

	void SetAnimInstance(UAnimInstance* NewAnimInstance);
	void SetDebugType(EDrawDebugTrace::Type NewDebugTrace);
	
protected:

	UPROPERTY()
	TObjectPtr<ACharacter> CharacterAvatar;
	
	UPROPERTY()
	TObjectPtr<UChooserTable> ChooserTable;

	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;
	
	float CapsuleRadius = 0.f;
	float CapsuleHalfHeight = 0.f;
	FVector ActorLocation = FVector::ZeroVector;
	FVector FrontLedgeLocationCheck = FVector::ZeroVector;
	FVector BackLedgeLocationCheck = FVector::ZeroVector;
	TEnumAsByte<ETraceTypeQuery> TraversalTraceChannel;
	
	int32 DrawDebugLevel = 0;
	float DrawDebugDuration = 0.f;
	EDrawDebugTrace::Type DebugTrace = EDrawDebugTrace::None; 
	
	/** Initializes all values that will be used by the instance. */
	virtual bool InitializeValues();
	
	/** Determines all viable inputs based on the avatar state. */
	virtual TOptional<FCharacterMovementTraversalCheckInputs> GetTraversalCheckInputs() const;

	/** Scans for a primitive component in front of the avatar. */
	virtual UPrimitiveComponent* ScanForPrimitiveComponent(const FCharacterMovementTraversalCheckInputs& Inputs) const;

	/** Starts calculating ledge data for both ends of a given traversable actor. */
	virtual void CalculateLedges(const FCharacterMovementTraversalCheckInputs& Inputs, FCharacterMovementTraversalCheckResult& InOutResult);

	/** Checks if there is enough room on top of the obstacle so the character can climb. */
	virtual bool CheckSpaceAboveObstacle(FCharacterMovementTraversalCheckResult& InOutResult);

	/** Checks if there is enough room across the obstacle so the character can cross. */
	virtual bool CheckSpaceAcrossObstacle(FCharacterMovementTraversalCheckResult& InOutResult);

	/** Checks if there is a floor after the obstacle, so we know if we are landing or falling. */
	virtual bool CheckFloorBehindObstacle(FCharacterMovementTraversalCheckResult& InOutResult);

	/** Selects the proper animation from the Chooser Table, based on the results. */
	virtual bool ChooseAnimationForTraversalAction(FCharacterMovementTraversalCheckResult& InOutResult);

};
