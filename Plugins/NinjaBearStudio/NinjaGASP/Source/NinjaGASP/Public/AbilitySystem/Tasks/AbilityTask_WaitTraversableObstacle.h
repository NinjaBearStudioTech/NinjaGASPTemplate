// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Types/FCharacterMovementTraversalCheckInputs.h"
#include "Types/FCharacterMovementTraversalCheckResult.h"
#include "AbilityTask_WaitTraversableObstacle.generated.h"

class ACharacter;
class UAnimInstance;
class UChooserTable;

/**
 * Tries to find a traversable object and broadcasts the result. 
 */
UCLASS()
class NINJAGASP_API UAbilityTask_WaitTraversableObstacle : public UAbilityTask
{

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTraversableObstacleFoundMCDelegate, const FCharacterMovementTraversalCheckResult&, Result);
	
	GENERATED_BODY()

public:

	/** A traversable obstacle has been found. */
	UPROPERTY(BlueprintAssignable)	
	FTraversableObstacleFoundMCDelegate OnTraversableObstacleSearchFinished;

	/**
	 * Tries to find a traversable object and broadcasts results.
	 */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability|Tasks")
	static UAbilityTask_WaitTraversableObstacle* CreateTask(UGameplayAbility* OwningAbility, ECollisionChannel TraversalTraceChannel, UChooserTable* TraversalChooserTable, TSubclassOf<UAbilityTask_WaitTraversableObstacle> TaskClass = nullptr);

	// -- Begin Ability Task implementation
	virtual void Activate() override;
	// -- End Ability Task implementation

	/**
	 * Option to set a specific Animation Instance, if the one provided by Combat is not adequate.
	 *
	 * Providing a null value has no effect and the internal logic to retrieve the instance will be used.
	 * Make sure to call this **before** activating the ability, so the Anim Instance is correctly set.
	 */
	void SetAnimInstance(UAnimInstance* NewAnimInstance);
	
protected:

	UPROPERTY()
	TObjectPtr<ACharacter> CharacterAvatar;
	
	UPROPERTY()
	TObjectPtr<UChooserTable> ChooserTable;

	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;

	// -----
	// Functions migrated from the actual GASP character.
	// -----
	
	/** Initializes all values that will be used by the instance. */
	virtual bool InitializeValues();
	
	/** Determines all viable inputs based on the avatar state. */
	virtual TOptional<FCharacterMovementTraversalCheckInputs> GetTraversalCheckInputs() const;

	/** Scans for a primitive component in front of the avatar. */
	virtual UPrimitiveComponent* ScanForPrimitiveComponent(const FCharacterMovementTraversalCheckInputs& Inputs, FHitResult& TraversalHit);

	/** Starts calculating ledge data for both ends of a given traversable actor. */
	virtual void TryAndCalculateLedges(FHitResult& OriginalHit, FCharacterMovementTraversalCheckResult& InOutResult);

	/** Checks if there is enough room on top of the obstacle so the character can climb. */
	virtual bool CheckSpaceAboveObstacle(FCharacterMovementTraversalCheckResult& InOutResult);

	/** Performs a sweep slightly above the top surface to check if the capsule can move from front to back ledge without collisions. */
	virtual bool PerformTopSweepAcrossObstacle(FCharacterMovementTraversalCheckResult& InOutResult);

	/** Checks if there is a floor after the obstacle, so we know if we are landing or falling. */
	virtual bool CheckFloorBehindObstacle(FCharacterMovementTraversalCheckResult& InOutResult);

	/** Selects the proper animation from the Chooser Table, based on the results. */
	virtual bool ChooseAnimationForTraversalAction(FCharacterMovementTraversalCheckResult& InOutResult);

	// -----
	// Functions migrated from the dynamic traversal component.
	// -----
	
	/** Determines all ledge data for an object, determining the proper traversal operation. */
	virtual void ComputeLedgeData(FHitResult& Hit, const FVector& Direction, bool& bFoundFrontLedge, bool& bFoundBackLedge, FVector& StartLedgeLocation, FVector& StartLedgeNormal, FVector& EndLedgeLocation, FVector& EndLedgeNormal) const;
	
	/** Nudges towards the centre of the object to avoid failed traces that hit right at the edge. */
	virtual void NudgeTraceTowardsObjectOrigin(FHitResult& Hit) const;

	/** Trace corners to determine if we need to move the character for better placement. */
	virtual void TraceCorners(const FHitResult& Hit, const FVector& TraceDirection, float TraceLength, bool& bCloseToCorner, float& DistanceToCorner, FVector& OffsetCornerPoint) const;

	/** Traces across the entire shape to determine the width. */
	virtual bool TraceWidth(const FHitResult& Hit, const FVector& Direction, FHitResult& OutHit) const;
	
	/** Traces along the plane to figure out what's the grab location. */
	virtual bool TraceAlongHitPlane(const FHitResult& Hit, const FVector& TraceDirection, float TraceLength, FHitResult& OutHit) const;
	
public:
	
	bool RespectPlayersFacingDir = false;
	float MinLedgeWidth = 70.f;
	float MinFrontLedgeDepth = 37.5f; 
	float MaxLedgeSidewaysDepth = 70.f;
	float CapsuleRadius = 0.f;
	float CapsuleHalfHeight = 0.f;
	FVector ActorLocation = FVector::ZeroVector;
	FVector FrontLedgeLocationCheck = FVector::ZeroVector;
	FVector BackLedgeLocationCheck = FVector::ZeroVector;
	TEnumAsByte<ECollisionChannel> TraversalTraceChannel;

	bool bShowTrace = false;
	bool bTraceComplex = true;
	bool bPersistentShowTrace = false;
	int32 DrawDebugLevel = 0;
	float DrawDebugDuration = 0.f;
	EDrawDebugTrace::Type DebugTrace = EDrawDebugTrace::None;
	
};
