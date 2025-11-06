// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNodeReference.h"
#include "Animation/TrajectoryTypes.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Types/EAnimationStateMachineState.h"
#include "Types/ECharacterGait.h"
#include "Types/ECharacterMovementMode.h"
#include "Types/ECharacterMovementState.h"
#include "Types/ECharacterRotationMode.h"
#include "Types/ECharacterStance.h"
#include "Types/EPlayerCameraMode.h"
#include "Types/FAnimationBlendStackInputs.h"
#include "Types/FAnimationStateMachineControlFlags.h"
#include "NinjaGASPAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;

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
class NINJAGASP_API UNinjaGASPAnimInstance : public UAnimInstance
{
	
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	bool bHasVelocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	bool bHasAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Essential Values")
	bool bJustLanded;
	
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
	
	UNinjaGASPAnimInstance();

	/**
	 * Checks if the character is considered moving, based on velocity and trajectory.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	bool IsMoving() const;

	/**
	 * Informs if gait changed since last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedGait() const { return Gait != GaitOnLastFrame; }

	/**
	 * Informs if the movement mode changed since last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedMovementMode() const { return MovementMode != MovementModeOnLastFrame; }

	/**
	 * Informs if the movement state changed since last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedMovementState() const { return MovementState != MovementStateOnLastFrame; }

	/**
	 * Informs if the rotation mode changed since last frame.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool ChangedRotationMode() const { return RotationMode != RotationModeOnLastFrame; }
	
	/**
	 * Informs if stance changed since last frame.
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
	bool HasLandedLight() const { return bJustLanded && FMath::Abs(LandedVelocity.Z) < FMath::Abs(HeavyLandSpeedThreshold); }

	/**
	 * Determines if the character landed above or at the heavy threshold (heavy land).
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	bool HasLandedHeavy() const { return bJustLanded && FMath::Abs(LandedVelocity.Z) >= FMath::Abs(HeavyLandSpeedThreshold); }

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
	 * Evaluates the requested state and then selects and updates the data to run it.
	 * Meant to be called from "On State Entry" functions.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe))
	void SetBlendStackAnimFromChooser(EAnimationStateMachineState NewState, bool bForceBlend);
	
	/**
	 * Provides an animation node that indicates the offset root transform.
	 * Usually, this is the "Offset Root Bone" node from the Anim Graph.
	 */
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "NBS|GASP|Animation Instance", meta = (BlueprintThreadSafe, ForceAsFunction))
	FAnimNodeReference GetOffsetRootNode() const ;

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
	
};
