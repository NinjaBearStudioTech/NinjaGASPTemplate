// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNodeReference.h"
#include "Animation/TrajectoryTypes.h"
#include "BoneControllers/AnimNode_OffsetRootBone.h"
#include "BoneControllers/AnimNode_OrientationWarping.h"
#include "Interfaces/TraversalActionTransformInterface.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Types/EAnimationStateMachineState.h"
#include "Types/ECharacterGait.h"
#include "Types/ECharacterMovementDirection.h"
#include "Types/ECharacterMovementDirectionBias.h"
#include "Types/ECharacterMovementMode.h"
#include "Types/ECharacterMovementState.h"
#include "Types/ECharacterRotationMode.h"
#include "Types/ECharacterStance.h"
#include "Types/EPlayerCameraMode.h"
#include "Types/FAnimationBlendStackInputs.h"
#include "Types/FAnimationStateMachineControlFlags.h"
#include "Types/FCharacterMovementDirectionThresholds.h"
#include "NinjaGASPAnimInstance.generated.h"

class ACharacter;
class UChooserTable;
class UPoseSearchDatabase;
class UCharacterMovementComponent;
class UAnimSequence;

/** Settings used with the Pose Search Trajectory functionality. */
USTRUCT(BlueprintType)
struct FPoseSearchGenerationSettings
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search Settings")
	float HistorySamplingInterval = -1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search Settings")
	int32 TrajectoryHistoryCount = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search Settings")
	float PredictionSamplingInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search Settings")
	int32 TrajectoryPredictionCount = 15;
	
};

/**
 * Lightweight data transfer object for the animation thread.
 *
 * Even though the Animation Instance class has a dedicated "UpdateOnThread" function, we want
 * to create a specific data type that can be used to transfer read-only data between the game
 * thread and the animation thread.
 */
USTRUCT()
struct FNinjaGASPAnimInstanceProxy : public FAnimInstanceProxy
{
	
	GENERATED_BODY()

	using Super = FAnimInstanceProxy;
	
	/**
	 * Character that is running the animation blueprint.
	 * Non-specific type, but should implement the appropriate interfaces.
	 */
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> CharacterOwner; 

	/**
	 * Character Movement Component assigned to the character.
	 * Non-specific type, but should implement the appropriate interfaces.
	 */
	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CharacterMovement;

	/**
	 * Checks if this proxy has been initialized with valid data.
	 * The proxy is supposed to be initialized by the framework, via "InitializeObjects".
	 */
	bool HasValidData() const;
	
protected:

	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	
};

/**
 * Base animation instance for the GASP project.
 *
 * Implements thread-safe updates and does not have a strong dependency on any character type.
 * 
 * All necessary data is obtained either from base classes (Character, Character Movement Component),
 * or from interfaces implemented by these objects. That way, you can have a more flexible character
 * hierarchy, potentially support variants for AI, Players, etc.
 */
UCLASS()
class NINJAGASP_API UNinjaGASPAnimInstance : public UAnimInstance, public ITraversalActionTransformInterface
{
	
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	bool bOffsetRootBoneEnabled;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	bool bHasVelocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	bool bHasAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	bool bJustLanded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	int32 MotionMatchingDatabaseLOD;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	float AccelerationAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	float MaximumAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	float MaximumBrakingDeceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	float Speed2D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	float MaximumSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FVector Acceleration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FVector AccelerationOnLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FVector VelocityOnLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FVector VelocityAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FVector LastNonZeroVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FVector LandedVelocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FTransform RootTransform;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FTransform CharacterTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	FTransform CharacterTransformOnLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	TArray<FName> CurrentDatabaseTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	TObjectPtr<const UPoseSearchDatabase> CurrentSelectedDatabase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Root Offset")
	float OffsetRootTranslationRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	bool bInRagdoll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
	float FlailRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	bool bIsAiming;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	FVector2D AimOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	EPlayerCameraMode CameraMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterMovementMode MovementMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterMovementMode MovementModeOnLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterRotationMode RotationMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterRotationMode RotationModeOnLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterMovementState MovementState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterMovementState MovementStateOnLastFrame;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterGait Gait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterGait GaitOnLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterStance Stance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
	ECharacterStance StanceOnLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FTransform TraversalTransform;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
	FPoseSearchTrajectoryData TrajectoryGenerationDataWhenIdle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
	FPoseSearchTrajectoryData TrajectoryGenerationDataWhenMoving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
	FTransformTrajectory Trajectory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
	FPoseSearchTrajectory_WorldCollisionResults TrajectoryCollisionResult;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
	float PreviousDesiredControllerYaw;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
	FVector TrajectoryPastVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
	FVector TrajectoryCurrentVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
	FVector TrajectoryFutureVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine")
	bool bUseStateMachine;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Anim Graph")
	EAnimationStateMachineState StateMachineState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Anim Graph")
	FAnimationBlendStackInputs BlendStackInputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Anim Graph")
	FAnimationBlendStackInputs PreviousBlendStackInputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Anim Graph")
	FAnimationStateMachineControlFlags AnimationControlFlags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Anim Graph")
	float SearchCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Movement Direction")
	ECharacterMovementDirection MovementDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Movement Direction")
	ECharacterMovementDirection MovementDirectionOnLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Movement Direction")
	ECharacterMovementDirectionBias MovementDirectionBias;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Movement Direction")
	FCharacterMovementDirectionThresholds DirectionThresholds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Target Rotation")
	FRotator TargetRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Target Rotation")
	FRotator TargetRotationOnTransitionStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine|Target Rotation")
	float TargetRotationDelta;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bUseVelocityToDetermineMovement = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bUseTrajectoryFutureVelocityToDetermineMovement = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bUseAccelerationToDetermineMovement = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float SpeedThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float TurnInPlaceYawThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float SpinTransitionYawThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float SpinTransitionSpeedThreshold;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float VelocityAccelerationThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float HeavyLandSpeedThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float TraverseYawThreshold;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float VelocityMovementTolerance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float TrajectoryFutureVelocityMovementTolerance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float AccelerationMovementTolerance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FPoseSearchGenerationSettings TransformTrajectorySettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TEnumAsByte<ETraceTypeQuery> TrajectoryCollisionTraceChannel = TraceTypeQuery1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FVector2D PastTrajectoryTimeSample;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FVector2D CurrentTrajectoryTimeSample;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FVector2D FutureTrajectoryTimeSample;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FName AimOffsetCurveName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UChooserTable> PoseSearchDatabaseTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UChooserTable> StateMachineAnimationTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	TObjectPtr<UAnimSequence> CurveAnimSequenceForStrafeOffset;
	
	UNinjaGASPAnimInstance();

	// -- Begin TraversalActionTransform implementation
	virtual FTransform GetTraversalTransform_Implementation() const override;
	virtual void SetTraversalTransform_Implementation(FTransform NewTransform) override;
	// -- End TraversalActionTransform implementation
	
	/**
	 * Checks if the character is considered moving, based on velocity and trajectory.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	bool IsMoving() const;

	/**
	 * Informs if gait changed from last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedGait() const { return Gait != GaitOnLastFrame; }

	/**
	 * Informs if the movement direction changed from last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedMovementDirection() const { return MovementDirection != MovementDirectionOnLastFrame; }
	
	/**
	 * Informs if the movement mode changed from last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedMovementMode() const { return MovementMode != MovementModeOnLastFrame; }

	/**
	 * Informs if the movement state changed from last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedMovementState() const { return MovementState != MovementStateOnLastFrame; }

	/**
	 * Informs if the rotation mode changed from last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedRotationMode() const { return RotationMode != RotationModeOnLastFrame; }
	
	/**
	 * Informs if stance changed from last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedStance() const { return Stance != StanceOnLastFrame; }
	
	/**
	 * Informs if the character is starting to move, based on velocity and trajectory future velocity.
	 * It also takes into consideration if there's an ongoing pivot animation.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool IsStartingToMove() const;
	
	/**
	 * Determines if the character should turn in place.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ShouldTurnInPlace() const;

	/**
	 * Allows a spin transition animation to play when the root bone rotation and character's capsule rotations are very different while moving.
	 * Spin transitions are locomotion animations that rotate the character while moving in a fixed world direction, and are useful when switching rotation modes. 
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ShouldSpinTransition() const;
	
	/**
	 * Determines if the character landed below the heavy threshold (light land).
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool HasLandedLight() const
	{
		return bJustLanded && FMath::Abs(LandedVelocity.Z) < FMath::Abs(HeavyLandSpeedThreshold);
	}

	/**
	 * Determines if the character landed above or at the heavy threshold (heavy land).
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool HasLandedHeavy() const
	{
		return bJustLanded && FMath::Abs(LandedVelocity.Z) >= FMath::Abs(HeavyLandSpeedThreshold);
	}

	/**
	 * Checks if the character just transitioned from "In Air" to "On Ground" movement, while standing.
	 * This transition is relevant to the State Machine setup, so it selects landing animations.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool TransitionedFromInAirToOnGround() const
	{
		return MovementMode == ECharacterMovementMode::OnGround && MovementModeOnLastFrame == ECharacterMovementMode::InAir;
	}

	/**
	 * Checks if the character just transitioned from "In Air" to "On Ground" movement, while moving.
	 * This transition is relevant to the State Machine setup, so it selects landing animations with movement.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool TransitionedFromInAirToOnGroundWhileMoving() const
	{
		return MovementMode == ECharacterMovementMode::OnGround && MovementModeOnLastFrame == ECharacterMovementMode::InAir
			&& FMath::Abs(GetTrajectoryTurnAngle()) <= 120.f;
	}
	
	/**
	 * Evaluates/selects the tail end of traversal animations when blending back to locomotion.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool JustTraversed() const;

	/**
	 * Checks if the character is pivoting.
	 * 
	 * This supports different checks for Motion Matching and State Machine modes. You can modify
	 * the logic for each check individually, by extending their specific pivoting functions. 
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool IsPivoting() const;

	/**
	 * Defines the aim offset state, considering the strafe mode, movement thresholds and montages playing.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ShouldEnableAimOffset() const;

	/**
	 * Provides the velocity on Z axis, when landing.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	float GetLandVelocity() const { return LandedVelocity.Z; }
	
	/**
	 * Provides the Yaw part of the Aim Offset, considering the rotation mode.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	float GetAimOffsetYaw() const
	{
		return RotationMode == ECharacterRotationMode::Strafe ? AimOffset.X : 0.f;	
	}

	/**
	 * Gets the direction between the future velocity and the current character rotation,
	 * and uses a curve to map that direction to an offset value
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	float GetStrafeYawRotationOffset() const;

	/**
	 * Provides the blend time for a Motion Matching node, based on current and previous states.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	float GetMotionMatchingBlendTime() const;

	/**
	 * Controls the speed at which the Root Offset node can interpolate the root bone's translation
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	float GetOffsetRootTranslationHalfLife() const;

	/**
	 * Retrieves the Offset Root Node's "Max Translation Error" radius from a console variable.
	 * This makes it easy to tune while playing.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	float GetOffsetRootTranslationRadius() const { return OffsetRootTranslationRadius; }
	
	/**
	 * Determines the Offset Root Rotation mode based on internal state.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootRotationMode() const;

	/**
	 * Determines the Offset Root Translation mode.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootTranslationMode() const;

	/**
	 * Defines the Orientation Warping Space, based on the root bone offset state.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	EOrientationWarpingSpace GetOrientationWarpingSpace() const;
	
	/**
	 * Determines whether motion matching will force a blend into a new database or wait until it finds a better match.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	EPoseSearchInterruptMode GetMotionMatchingInterruptMode() const;

protected:

	/** Proxy that transfers data between the game and animation threads. */
	UPROPERTY(Transient)
	FNinjaGASPAnimInstanceProxy Proxy;

	// -- Begin AnimInstance implementation
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	// -- End AnimInstance implementation

	/**
	 * Updates all trajectory values and collisions.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void UpdateTrajectory(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement);

	/**
	 * Updates all essential values, using the character and movement component.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void UpdateEssentialValues(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement);

	/**
	 * Updates all state enums that are used internally and by chooser tables.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void UpdateStates(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement);

	/**
	 * Updates the aiming state and aim offset vector.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void UpdateAiming(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement);

	/**
	 * Updates details about the ragdoll state.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void UpdateRagdoll(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement);

	/**
	 * Updates movement direction for the State Machine.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void UpdateMovementDirection(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement);

	/**
	 * Updates target rotation for the StateMachine.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void UpdateTargetRotation(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement);
	
	/**
	 * Updates motion matching data, selecting the database using chooser tables.
	 * To extend this functionality, please look into "HandleMotionMatching".
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void UpdateMotionMatching(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	
	/**
	 * Updates motion matching data, after a node has been selected in animation.
	 * To extend this functionality, please look into "HandleMotionMatchingPostSelection".
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void UpdateMotionMatchingPostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/**
	 * Handles the FSM entering the "Idle Loop" state entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance|State Machine|Anim Graph", meta = (BlueprintThreadSafe))
	void OnStateEntry_IdleLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/**
	 * Handles the FSM entering the "Transition to Idle Loop" state entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance|State Machine|Anim Graph", meta = (BlueprintThreadSafe))
	void OnStateEntry_TransitionToIdleLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/**
	 * Handles the FSM entering the "Locomotion Loop" state entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance|State Machine|Anim Graph", meta = (BlueprintThreadSafe))
	void OnStateEntry_LocomotionLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	
	/**
	 * Handles the FSM entering the "Transition to Locomotion Loop" state entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance|State Machine|Anim Graph", meta = (BlueprintThreadSafe))
	void OnStateEntry_TransitionToLocomotionLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/**
	 * Handles the FSM entering the "In Air Loop" state entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance|State Machine|Anim Graph", meta = (BlueprintThreadSafe))
	void OnStateEntry_InAirLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/**
	 * Handles the FSM entering the "Transition to Air Loop" state entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance|State Machine|Anim Graph", meta = (BlueprintThreadSafe))
	void OnStateEntry_TransitionToInAirLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/**
	 * Handles the FSM entering the "Idle Break" state entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance|State Machine|Anim Graph", meta = (BlueprintThreadSafe))
	void OnStateEntry_IdleBreak(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	
	/**
	 * Handles the FSM entering the "Transition to Locomotion Loop" state update
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance|State Machine|Anim Graph", meta = (BlueprintThreadSafe))
	void OnUpdate_TransitionToLocomotionLoop(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	
	/**
	 * Evaluates the requested state and then selects and updates the data to run it.
	 * Meant to be called from "On State Entry" functions.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	void SetBlendStackAnimFromChooser(EAnimationStateMachineState NewState, bool bForceBlend);

	/**
	 * Used in the State Controller to trigger a blend whenever the currently playing animation is ending.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool IsBlendStackAnimationAlmostComplete() const;
	
	/**
	 * Provides an animation node that indicates the offset root transform.
	 * Usually, this is the "Offset Root Bone" node from the Anim Graph.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	FAnimNodeReference GetOffsetRootNode() const;

	/**
	 * Provides an animation node that is responsible for the blend stack.
	 * Usually, this is the "Offset Root Bone" node from the Anim Graph.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	FAnimNodeReference GetStateMachineBlendStackNode() const;
	
	/**
	 * Provides all thresholds applied to movement, while in State Machine mode.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	FCharacterMovementDirectionThresholds GetMovementDirectionThresholds() const;
	
	/**
	 * Gets the turn angle expected from the current trajectory.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	float GetTrajectoryTurnAngle() const;
	
	/**
	 * Calculates the Root Transform for the character.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	FTransform CalculateRootTransform(const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement) const;

	/**
	 * Calculates the aim offset based on current values.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	FVector2D CalculateAimOffset(const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement) const;
	
	/**
	 * Converts the CMC Movement Mode to the custom movement mode enum.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	ECharacterMovementMode ConvertCharacterMovementMode(const UCharacterMovementComponent* CharacterMovement) const;

	/**
	 * Converts CMC rotation flags to the custom rotation mode enum.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	ECharacterRotationMode ConvertCharacterRotationMode(const UCharacterMovementComponent* CharacterMovement) const;

	/**
	 * Checks if the character is pivoting, considering Motion Matching conditions.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	bool IsPivotingInMotionMatching() const;

	/**
	 * Checks if the character is pivoting, considering State Machine conditions.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	bool IsPivotingInStateMachine() const;

	/**
	 * Handles a motion matching update, executed **during** animation selection.
	 * This is usually called by "UpdateMotionMatching". 
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	void HandleMotionMatching(const FMotionMatchingAnimNodeReference& MotionMatchingNodeRef);
	
	/**
	 * Handles a motion matching update, executed **post** animation selection.
	 * This is usually called by "UpdateMotionMatchingPostSelection". 
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	void HandleMotionMatchingPostSelection(const FMotionMatchingAnimNodeReference& MotionMatchingNodeRef);
	
};
