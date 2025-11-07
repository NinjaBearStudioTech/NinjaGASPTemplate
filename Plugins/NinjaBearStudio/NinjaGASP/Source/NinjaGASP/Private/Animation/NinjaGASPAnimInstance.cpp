// Ninja Bear Studio Inc., all rights reserved.
#include "Animation/NinjaGASPAnimInstance.h"

#include "AnimationWarpingLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Animation/AnimationAsset.h"
#include "Animation/BlendProfile.h"
#include "BlendStack/BlendStackAnimNodeLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"
#include "Interfaces/PlayerCameraModeInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "PoseSearch/AnimNode_MotionMatching.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchResult.h"
#include "Types/FAnimationMotionMatchingChooserOutputs.h"

#pragma region Proxy
// Begin Proxy implementation -------------------------

bool FNinjaGASPAnimInstanceProxy::HasValidData() const
{
	return IsValid(CharacterOwner) && IsValid(CharacterMovement);
}

void FNinjaGASPAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	Super::InitializeObjects(InAnimInstance);

	APawn* PawnOwner = InAnimInstance->TryGetPawnOwner();
	if (IsValid(PawnOwner) && PawnOwner->IsA<ACharacter>() && PawnOwner->Implements<UAdvancedCharacterMovementInterface>())
	{
		CharacterOwner = Cast<ACharacter>(PawnOwner);
		CharacterMovement = CharacterOwner->GetCharacterMovement();	
	}
}

// End Proxy implementation -------------------------------
#pragma endregion Proxy

UNinjaGASPAnimInstance::UNinjaGASPAnimInstance()
{
	bHasVelocity = false;
	bJustLanded = false;
	bHasAcceleration = false;
	MotionMatchingDatabaseLOD = 0;
	AccelerationAmount = 0.f;
	MaximumAcceleration = 0.f;
	MaximumBrakingDeceleration = 0.f;
	Speed2D = 0.f;
	MaximumSpeed = 0.f;
	Velocity = FVector::ZeroVector;
	VelocityOnLastFrame = FVector::ZeroVector;
	VelocityAcceleration = FVector::ZeroVector;
	LastNonZeroVelocity = FVector::ZeroVector;
	LandedVelocity = FVector::ZeroVector;
	Acceleration = FVector::ZeroVector;
	AccelerationOnLastFrame = FVector::ZeroVector;
	RootTransform = FTransform::Identity;
	CharacterTransform = FTransform::Identity;
	CharacterTransformOnLastFrame = FTransform::Identity;
	CurrentSelectedDatabase = nullptr;

	bInRagdoll = false;
	FlailRate = 0.f;
	
	bIsAiming = false;
	AimOffset = FVector2D::ZeroVector;
	CameraMode = EPlayerCameraMode::Balanced;

	MovementMode = ECharacterMovementMode::OnGround;
	MovementModeOnLastFrame = ECharacterMovementMode::OnGround;
	RotationMode = ECharacterRotationMode::OrientToMovement;
	RotationModeOnLastFrame = ECharacterRotationMode::OrientToMovement;
	MovementState = ECharacterMovementState::Idle;
	MovementStateOnLastFrame = ECharacterMovementState::Idle;
	Gait = ECharacterGait::Run;
	GaitOnLastFrame = ECharacterGait::Run;
	Stance = ECharacterStance::Stand;
	StanceOnLastFrame = ECharacterStance::Stand;

	TrajectoryGenerationDataWhenIdle = FPoseSearchTrajectoryData();
	TrajectoryGenerationDataWhenIdle.RotateTowardsMovementSpeed = 0.f;
	TrajectoryGenerationDataWhenIdle.MaxControllerYawRate = 100.f;

	TrajectoryGenerationDataWhenMoving = FPoseSearchTrajectoryData();
	TrajectoryGenerationDataWhenMoving.RotateTowardsMovementSpeed = 0.f;
	TrajectoryGenerationDataWhenMoving.MaxControllerYawRate = 0.f;
	
	Trajectory = FTransformTrajectory();
	TrajectoryCollisionResult = FPoseSearchTrajectory_WorldCollisionResults();

	PreviousDesiredControllerYaw = 0.f;
	TrajectoryPastVelocity = FVector::ZeroVector;
	TrajectoryCurrentVelocity = FVector::ZeroVector;
	TrajectoryFutureVelocity = FVector::ZeroVector;

	bUseStateMachine = false;
	SearchCost = 0.f;
	StateMachineState = EAnimationStateMachineState::IdleLoop;
	BlendStackInputs = FAnimationBlendStackInputs();
	PreviousBlendStackInputs = FAnimationBlendStackInputs();
	AnimationControlFlags = FAnimationStateMachineControlFlags();
	MovementDirection = ECharacterMovementDirection::F;
	MovementDirectionOnLastFrame = ECharacterMovementDirection::F;
	MovementDirectionBias = ECharacterMovementDirectionBias::LeftFootForward;
	DirectionThresholds = FCharacterMovementDirectionThresholds();
	TargetRotation = FRotator::ZeroRotator;
	TargetRotationOnTransitionStart = FRotator::ZeroRotator;
	TargetRotationDelta = 0.f;
	
	bUseVelocityToDetermineMovement = false;
	bUseTrajectoryFutureVelocityToDetermineMovement = true;
	bUseAccelerationToDetermineMovement = true;
	SpeedThreshold = 5.f;
	TurnInPlaceYawThreshold = 50.f;
	SpinTransitionYawThreshold = 130.f;
	SpinTransitionSpeedThreshold = 150.f;
	VelocityAccelerationThreshold = 0.001f;
	TraverseYawThreshold = 50.;
	HeavyLandSpeedThreshold = 700.f;
	VelocityMovementTolerance = 0.1f;
	TrajectoryFutureVelocityMovementTolerance = 10.f;
	AccelerationMovementTolerance = 0.f;
	TransformTrajectorySettings = FPoseSearchGenerationSettings();
	TrajectoryCollisionTraceChannel = TraceTypeQuery1;
	PastTrajectoryTimeSample = FVector2D(-0.3f, -0.2f);
	CurrentTrajectoryTimeSample = FVector2D(0.f, 0.2f);
	FutureTrajectoryTimeSample = FVector2D(0.4f, 0.5f);
	AimOffsetCurveName = TEXT("Disable_AO");
}

FAnimInstanceProxy* UNinjaGASPAnimInstance::CreateAnimInstanceProxy()
{
	return &Proxy;
}

void UNinjaGASPAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	// We don't want to call super here, since we actually want to keep the proxy.
}

void UNinjaGASPAnimInstance::NativeThreadSafeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Proxy.HasValidData()) return;

	UpdateEssentialValues(DeltaSeconds,Proxy.CharacterOwner, Proxy.CharacterMovement);
	UpdateAiming(DeltaSeconds,Proxy.CharacterOwner, Proxy.CharacterMovement);
	UpdateTrajectory(DeltaSeconds,Proxy.CharacterOwner, Proxy.CharacterMovement);
	UpdateStates(DeltaSeconds,Proxy.CharacterOwner, Proxy.CharacterMovement);
	UpdateRagdoll(DeltaSeconds,Proxy.CharacterOwner, Proxy.CharacterMovement);

	if (bUseStateMachine)
	{
		UpdateMovementDirection(DeltaSeconds,Proxy.CharacterOwner, Proxy.CharacterMovement);
		UpdateTargetRotation(DeltaSeconds,Proxy.CharacterOwner, Proxy.CharacterMovement);
	}
}

void UNinjaGASPAnimInstance::UpdateEssentialValues_Implementation(const float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement)
{
	CharacterTransformOnLastFrame = CharacterTransform;
	CharacterTransform = CharacterOwner->GetActorTransform();
	RootTransform = CalculateRootTransform(CharacterOwner, CharacterMovement);

	AccelerationOnLastFrame = Acceleration;
	Acceleration = CharacterMovement->GetCurrentAcceleration();
	MaximumAcceleration = CharacterMovement->GetMaxAcceleration();
	MaximumBrakingDeceleration = CharacterMovement->GetMaxBrakingDeceleration();
	AccelerationAmount = Acceleration.Size() / CharacterMovement->GetMaxAcceleration();
	bHasAcceleration = AccelerationAmount > 0.f; 

	VelocityOnLastFrame = Velocity;
	Velocity = CharacterMovement->Velocity;
	Speed2D = Velocity.Size2D();
	MaximumSpeed = CharacterMovement->GetMaxSpeed();
	VelocityAcceleration = (Velocity - VelocityOnLastFrame) / FMath::Max(DeltaSeconds, VelocityAccelerationThreshold);
	
	bHasVelocity = Speed2D > SpeedThreshold;
	if (bHasVelocity)
	{
		LastNonZeroVelocity = Velocity;
	}

	LandedVelocity = IAdvancedCharacterMovementInterface::Execute_GetLandVelocity(CharacterOwner);
	bJustLanded = IAdvancedCharacterMovementInterface::Execute_HasJustLanded(CharacterOwner);
}

FTransform UNinjaGASPAnimInstance::CalculateRootTransform_Implementation(const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement) const
{
	const FAnimNodeReference Node = GetOffsetRootNode();
	if (!Node.GetAnimNodePtr<FAnimNode_Base>())
	{
		// We don't have a valid node, let's use the actor transform.
		return CharacterOwner->GetActorTransform();	
	}
	
	const FTransform OffsetRootTransform = UAnimationWarpingLibrary::GetOffsetRootTransform(Node);
	const FRotator OffsetRootRotator = FRotator(OffsetRootTransform.Rotator().Pitch, OffsetRootTransform.Rotator().Yaw + 90.f, OffsetRootTransform.Rotator().Roll);
		
	FTransform Result;
	Result.SetLocation(OffsetRootTransform.GetLocation());
	Result.SetRotation(OffsetRootRotator.Quaternion());
	Result.SetScale3D(FVector::OneVector);
	return Result;
}

FAnimNodeReference UNinjaGASPAnimInstance::GetOffsetRootNode_Implementation() const
{
	return FAnimNodeReference();
}

void UNinjaGASPAnimInstance::UpdateAiming_Implementation(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement)
{
	bIsAiming = IAdvancedCharacterMovementInterface::Execute_IsAiming(CharacterOwner);
	AimOffset = CalculateAimOffset(CharacterOwner, CharacterMovement);

	if (CharacterOwner->Implements<UPlayerCameraModeInterface>())
	{
		CameraMode = IPlayerCameraModeInterface::Execute_GetCameraMode(CharacterOwner);
	}
}

FVector2D UNinjaGASPAnimInstance::CalculateAimOffset_Implementation(const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement) const
{
	const FRotator AimRotation = CharacterOwner->IsLocallyControlled() ? CharacterOwner->GetControlRotation() : CharacterOwner->GetBaseAimRotation();
	const FRotator RootRotation = RootTransform.Rotator();
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, RootRotation);

	const float Alpha = GetCurveValue(AimOffsetCurveName);
	const FVector LerpVector = UKismetMathLibrary::VLerp(FVector(DeltaRotation.Yaw, DeltaRotation.Pitch, 0.f), FVector::ZeroVector, Alpha);
	return FVector2D(LerpVector.X, LerpVector.Y);
}

void UNinjaGASPAnimInstance::UpdateTrajectory_Implementation(const float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement)
{
PRAGMA_DISABLE_EXPERIMENTAL_WARNINGS
	
	const float InHistorySamplingInterval = TransformTrajectorySettings.HistorySamplingInterval;
	const float InTrajectoryHistoryCount = TransformTrajectorySettings.TrajectoryHistoryCount;
	const float InPredictionSamplingInterval = TransformTrajectorySettings.PredictionSamplingInterval;
	const float InTrajectoryPredictionCount = TransformTrajectorySettings.TrajectoryPredictionCount;
	FTransformTrajectory NewTrajectory;
	
	const FPoseSearchTrajectoryData Data = Speed2D > 0.f ? TrajectoryGenerationDataWhenMoving : TrajectoryGenerationDataWhenIdle;
	const TArray<AActor*> IgnoredActors;
	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTransformTrajectory(this, Data, DeltaSeconds, Trajectory, PreviousDesiredControllerYaw, NewTrajectory,
		InHistorySamplingInterval, InTrajectoryHistoryCount, InPredictionSamplingInterval, InTrajectoryPredictionCount);

	static constexpr bool bApplyGravity = true;
	static constexpr bool bTraceComplex = false;
	static constexpr bool bIgnoreSelf = true;
	static constexpr float FloorCollisionOffset = 0.01f;
	static constexpr float MaxObstacleHeight = 150.f;
	
	UPoseSearchTrajectoryLibrary::HandleTransformTrajectoryWorldCollisions(CharacterOwner, this, NewTrajectory, bApplyGravity, FloorCollisionOffset, Trajectory,
		TrajectoryCollisionResult, TrajectoryCollisionTraceChannel, bTraceComplex, IgnoredActors, EDrawDebugTrace::None, bIgnoreSelf, MaxObstacleHeight);

	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, PastTrajectoryTimeSample.X, PastTrajectoryTimeSample.Y, TrajectoryPastVelocity);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, CurrentTrajectoryTimeSample.X, CurrentTrajectoryTimeSample.Y, TrajectoryCurrentVelocity);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(Trajectory, FutureTrajectoryTimeSample.X, FutureTrajectoryTimeSample.Y, TrajectoryFutureVelocity);
	
PRAGMA_ENABLE_EXPERIMENTAL_WARNINGS
}

void UNinjaGASPAnimInstance::UpdateStates_Implementation(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement)
{
	MovementModeOnLastFrame = MovementMode;
	MovementMode = ConvertCharacterMovementMode(CharacterMovement);

	RotationModeOnLastFrame = RotationMode;
	RotationMode = ConvertCharacterRotationMode(CharacterMovement);

	MovementStateOnLastFrame = MovementState;
	MovementState = IsMoving() ? ECharacterMovementState::Moving : ECharacterMovementState::Idle;

	GaitOnLastFrame = Gait;
	Gait = IAdvancedCharacterMovementInterface::Execute_GetCharacterGait(CharacterOwner);

	StanceOnLastFrame = Stance;
	Stance = CharacterOwner->IsCrouched() ? ECharacterStance::Crouch : ECharacterStance::Stand;
}

ECharacterMovementMode UNinjaGASPAnimInstance::ConvertCharacterMovementMode_Implementation(const UCharacterMovementComponent* CharacterMovement) const
{
	const EMovementMode& MovementModeFromCMC = CharacterMovement->MovementMode;
	switch (MovementModeFromCMC)
	{
		case MOVE_None:
		case MOVE_Walking:
		case MOVE_NavWalking:
		case MOVE_Flying:
		case MOVE_Custom:
			return ECharacterMovementMode::OnGround;
		case MOVE_Swimming:
			return ECharacterMovementMode::Swimming;
		case MOVE_Falling:
			return ECharacterMovementMode::InAir;
		default:
			return ECharacterMovementMode::OnGround;
	}
}

ECharacterRotationMode UNinjaGASPAnimInstance::ConvertCharacterRotationMode_Implementation(const UCharacterMovementComponent* CharacterMovement) const
{
	const bool bOrientToMovement = CharacterMovement->bOrientRotationToMovement;
	return bOrientToMovement ? ECharacterRotationMode::OrientToMovement : ECharacterRotationMode::Strafe;
}

bool UNinjaGASPAnimInstance::IsMoving_Implementation() const
{
	// Lambda helper that can perform the conditional check and apply the tolerance.
	auto HasMovement = [](const bool bUse, const FVector& V, const float Tolerance)
	{
		return !bUse || UKismetMathLibrary::NotEqual_VectorVector(V, FVector::ZeroVector, Tolerance);
	};

	return HasMovement(bUseVelocityToDetermineMovement, Velocity, VelocityMovementTolerance)
		&& HasMovement(bUseTrajectoryFutureVelocityToDetermineMovement, TrajectoryFutureVelocity, TrajectoryFutureVelocityMovementTolerance)
		&& HasMovement(bUseAccelerationToDetermineMovement, Acceleration, AccelerationMovementTolerance);
}

void UNinjaGASPAnimInstance::UpdateRagdoll_Implementation(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement)
{
	bInRagdoll = IAdvancedCharacterMovementInterface::Execute_InRagdoll(CharacterOwner);

	const float RawFlailRate = CharacterOwner->GetMesh()->GetPhysicsLinearVelocity().Size();
	FlailRate = UKismetMathLibrary::MapRangeClamped(RawFlailRate, 0.f, 1000.f, 0.f, 1.f);
}

void UNinjaGASPAnimInstance::UpdateMovementDirection_Implementation(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement)
{
	MovementDirectionOnLastFrame = MovementDirection;
	if (MovementState != ECharacterMovementState::Moving)
	{
		return;
	}

	static constexpr float Tolerance = 0.0001f;
	TrajectoryFutureVelocity.Normalize(Tolerance);
	DirectionThresholds = GetMovementDirectionThresholds();
	
	const FRotator BaseRotation = CharacterTransform.GetRotation().Rotator();
	const float Direction = UKismetAnimationLibrary::CalculateDirection(TrajectoryFutureVelocity, BaseRotation);

	if (RotationMode == ECharacterRotationMode::OrientToMovement || Gait == ECharacterGait::Sprint)
	{
		MovementDirection = ECharacterMovementDirection::F;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(Direction, DirectionThresholds.ForwardLeft, DirectionThresholds.ForwardRight))
	{
		MovementDirection = ECharacterMovementDirection::F;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(Direction, DirectionThresholds.BackwardLeft, DirectionThresholds.ForwardLeft))
	{
		MovementDirection = MovementDirectionBias == ECharacterMovementDirectionBias::LeftFootForward ?
			ECharacterMovementDirection::LL : ECharacterMovementDirection::LR;		
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(Direction, DirectionThresholds.ForwardRight, DirectionThresholds.BackwardRight))
	{
		MovementDirection = MovementDirectionBias == ECharacterMovementDirectionBias::LeftFootForward ?
			ECharacterMovementDirection::RL : ECharacterMovementDirection::RR;
	}
	else
	{
		MovementDirection = ECharacterMovementDirection::B;
	}
}

void UNinjaGASPAnimInstance::UpdateTargetRotation_Implementation(float DeltaSeconds, const ACharacter* CharacterOwner, const UCharacterMovementComponent* CharacterMovement)
{
	if (IsMoving())
	{
		switch (RotationMode)
		{
			case ECharacterRotationMode::OrientToMovement:
			{
				TargetRotation = CharacterTransform.Rotator();
				break;
			}
			case ECharacterRotationMode::Strafe:
			{
				TargetRotation.Roll = CharacterTransform.Rotator().Roll;
				TargetRotation.Roll = CharacterTransform.Rotator().Pitch;

				// When in the Strafe rotation mode, an offset is applied to the target rotation in
				// order for the directional animations to match the actual strafing angle.
				const float StrafeYawRotationOffset = GetStrafeYawRotationOffset();
				TargetRotation.Yaw = CharacterTransform.Rotator().Pitch + StrafeYawRotationOffset;
				break;
			}
		}
	}
	else
	{
		TargetRotation = CharacterTransform.Rotator();
	}

	const FRotator RootRotation = RootTransform.Rotator();
	TargetRotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, RootRotation).Yaw;
}

void UNinjaGASPAnimInstance::UpdateMotionMatching(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FMotionMatchingAnimNodeReference& MotionMatchingNodeRef = UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNode(Node, Result);
	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		HandleMotionMatching(MotionMatchingNodeRef);
	}	
}

void UNinjaGASPAnimInstance::HandleMotionMatching_Implementation(const FMotionMatchingAnimNodeReference& MotionMatchingNodeRef)
{
	if (IsValid(PoseSearchDatabaseTable))
	{
		UPoseSearchDatabase* Database = Cast<UPoseSearchDatabase>(UChooserFunctionLibrary::EvaluateChooser(this, PoseSearchDatabaseTable, UPoseSearchDatabase::StaticClass()));
		if (IsValid(Database))
		{
			const EPoseSearchInterruptMode InterruptMode = GetMotionMatchingInterruptMode();
			UMotionMatchingAnimNodeLibrary::SetDatabaseToSearch(MotionMatchingNodeRef, Database, InterruptMode);
		}	
	}
}

void UNinjaGASPAnimInstance::UpdateMotionMatchingPostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FMotionMatchingAnimNodeReference& MotionMatchingNodeRef = UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNode(Node, Result);
	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		HandleMotionMatchingPostSelection(MotionMatchingNodeRef);
	}
}

void UNinjaGASPAnimInstance::HandleMotionMatchingPostSelection_Implementation(const FMotionMatchingAnimNodeReference& MotionMatchingNodeRef)
{
	FPoseSearchBlueprintResult Result;
	bool bIsValidSearchResult;
	
	UMotionMatchingAnimNodeLibrary::GetMotionMatchingSearchResult(MotionMatchingNodeRef, Result, bIsValidSearchResult);
	if (bIsValidSearchResult)
	{
		CurrentSelectedDatabase = Result.SelectedDatabase;
		if (IsValid(CurrentSelectedDatabase))
		{
			CurrentDatabaseTags = CurrentSelectedDatabase->Tags;
		}
	}
}

bool UNinjaGASPAnimInstance::IsStartingToMove() const
{
	static const FName PivotsTag = FName("Pivots");

	const bool bTendsToIncreaseVelocity = TrajectoryFutureVelocity.Size2D() >= Velocity.Size2D() + 100;
	return IsMoving() && bTendsToIncreaseVelocity && !CurrentDatabaseTags.Contains(PivotsTag);
}

bool UNinjaGASPAnimInstance::ShouldTurnInPlace_Implementation() const
{
	const float NormalizedYaw = UKismetMathLibrary::NormalizedDeltaRotator(CharacterTransform.Rotator(), RootTransform.Rotator()).Yaw;
	if (FMath::Abs(NormalizedYaw) >= TurnInPlaceYawThreshold)
	{
		const bool bInFirstPerson = CameraMode == EPlayerCameraMode::FirstPerson;
		const bool bHasValidMovementState = MovementState == ECharacterMovementState::Idle && MovementStateOnLastFrame == ECharacterMovementState::Moving;
		
		if (bIsAiming || bInFirstPerson ||bHasValidMovementState) 
		{
			return true;
		}
	}

	return false;
}

bool UNinjaGASPAnimInstance::ShouldSpinTransition_Implementation() const
{
	static const FName PivotsTag = FName("Pivots");
	
	const float NormalizedYaw = UKismetMathLibrary::NormalizedDeltaRotator(CharacterTransform.Rotator(), RootTransform.Rotator()).Yaw;
	return FMath::Abs(NormalizedYaw) >= SpinTransitionYawThreshold && Speed2D >= SpinTransitionSpeedThreshold && !CurrentDatabaseTags.Contains(PivotsTag);
}

bool UNinjaGASPAnimInstance::JustTraversed() const
{
	static const FName DefaultSlot = FName("DefaultSlot");
	const bool bDefaultSlotActive = IsSlotActive(DefaultSlot);
	
	static const FName MovingTraversal = FName("MovingTraversal");
	const bool bHasMovingTraversalCurveValue = GetCurveValue(MovingTraversal) > 0.f;

	const float TrajectoryTurnAngle = GetTrajectoryTurnAngle();
	const bool TurnAngleBelowThreshold = TrajectoryTurnAngle <= TraverseYawThreshold; 

	return !bDefaultSlotActive && bHasMovingTraversalCurveValue && TurnAngleBelowThreshold;
}

FCharacterMovementDirectionThresholds
UNinjaGASPAnimInstance::GetMovementDirectionThresholds_Implementation() const
{
	static const FCharacterMovementDirectionThresholds ForwardBackwardThresholds(-60.f, 60.f, -120.f, 120.f);
	static const FCharacterMovementDirectionThresholds LateralLoopThresholds(-60.f, 60.f, -140.f, 140.f);
	static const FCharacterMovementDirectionThresholds LateralAimThresholds(-40.f, 40.f, -140.f, 140.f);

	const bool bIsForwardOrBackward =
		MovementDirection == ECharacterMovementDirection::F ||
		MovementDirection == ECharacterMovementDirection::B;

	const bool bIsLateral =
		MovementDirection == ECharacterMovementDirection::LL ||
		MovementDirection == ECharacterMovementDirection::LR ||
		MovementDirection == ECharacterMovementDirection::RL ||
		MovementDirection == ECharacterMovementDirection::RR;

	if (bIsForwardOrBackward)
	{
		return ForwardBackwardThresholds;
	}

	if (!bIsLateral)
	{
		return FCharacterMovementDirectionThresholds();
	}

	if (IsPivoting())
	{
		return ForwardBackwardThresholds;
	}

	const bool bUseLoopThresholds = BlendStackInputs.bLoop && !bIsAiming;
	return bUseLoopThresholds ? LateralLoopThresholds : LateralAimThresholds;
}

FAnimNodeReference UNinjaGASPAnimInstance::GetStateMachineBlendStackNode_Implementation() const
{
	return FAnimNodeReference();
}

float UNinjaGASPAnimInstance::GetTrajectoryTurnAngle() const
{
	const FRotator FutureVelocityRotator = TrajectoryFutureVelocity.ToOrientationRotator();
	const FRotator VelocityRotator = Velocity.ToOrientationRotator();
	return UKismetMathLibrary::NormalizedDeltaRotator(FutureVelocityRotator, VelocityRotator).Yaw;
}

bool UNinjaGASPAnimInstance::IsPivoting() const
{
	if (!IsMoving())
	{
		// If not moving, don't bother wasting cpu checking for conditions.
		return false;
	}

	return bUseStateMachine ? IsPivotingInStateMachine() : IsPivotingInMotionMatching(); 
}

bool UNinjaGASPAnimInstance::ShouldEnableAimOffset() const
{
	if (RotationMode != ECharacterRotationMode::Strafe)
	{
		return false;
	}

	const float Threshold = MovementState == ECharacterMovementState::Idle ? 115.f : 180.f; 
	if (FMath::Abs(AimOffset.X) > Threshold)
	{
		return false;
	}

	static const FName SlotName = TEXT("DefaultSlot");
	const float DefaultSlotWeight = Blueprint_GetSlotMontageLocalWeight(SlotName);
	if (DefaultSlotWeight >= 0.5f)
	{
		return false;
	}

	return true;
}

float UNinjaGASPAnimInstance::GetStrafeYawRotationOffset() const
{
	FVector NormalizedVelocity = TrajectoryFutureVelocity;
	NormalizedVelocity.Normalize();

	const FRotator BaseRotation = CharacterTransform.Rotator();
	const float Direction = UKismetAnimationLibrary::CalculateDirection(NormalizedVelocity, BaseRotation);

	// Explanation from the author of GASP:
	//
	// Because querying a curve asset is not currently thread safe, we store the curves in a dummy anim sequence,
	// which is why we have to map the direction to the number of frames the dummy anim sequence has, and divide by 30.
	// In the animation, each frame represents an angle at an increment of 45 degrees.
	//
	// Note from myself:
	// Yikes...
	//
	const float MappedDirection = UKismetMathLibrary::MapRangeClamped(Direction, -180.f, 180.f, 0.f, 8.f) / 30.f;

	FName CurveName = NAME_None;
	switch (MovementDirection)
	{
		case ECharacterMovementDirection::F:
			CurveName = TEXT("StrafeOffset_F");
			break;
		case ECharacterMovementDirection::B:
			CurveName = TEXT("StrafeOffset_B");
			break;
		case ECharacterMovementDirection::LL:
			CurveName = TEXT("StrafeOffset_LL");
			break;
		case ECharacterMovementDirection::LR:
			CurveName = TEXT("StrafeOffset_LR");
			break;
		case ECharacterMovementDirection::RL:
			CurveName = TEXT("StrafeOffset_RL");
			break;
		case ECharacterMovementDirection::RR:
			CurveName = TEXT("StrafeOffset_RR");
			break;
	}

	float OutValue = 0.f;
	
	if (IsValid(CurveAnimSequenceForStrafeOffset) && CurveName != NAME_None)
	{
		UAnimationWarpingLibrary::GetCurveValueFromAnimation(CurveAnimSequenceForStrafeOffset,
			CurveName, MappedDirection, OutValue);	
	}
	
	return OutValue;
}

EPoseSearchInterruptMode UNinjaGASPAnimInstance::GetMotionMatchingInterruptMode_Implementation() const
{
	if (ChangedMovementMode())
	{
		return EPoseSearchInterruptMode::InterruptOnDatabaseChange;
	}

	if (MovementMode == ECharacterMovementMode::OnGround)
	{
		if (ChangedMovementState() || ChangedGait() || ChangedStance())
		{
			return EPoseSearchInterruptMode::InterruptOnDatabaseChange;
		}
	}

	return EPoseSearchInterruptMode::DoNotInterrupt;
}

bool UNinjaGASPAnimInstance::IsPivotingInMotionMatching_Implementation() const
{
	float AngleThreshold = 0.f;
	switch (RotationMode)
	{
		case ECharacterRotationMode::OrientToMovement:
			AngleThreshold = 45.f;
			break;
		case ECharacterRotationMode::Strafe:
			AngleThreshold = 30.f;
			break;
	}
	
	const float TrajectoryTurnAngle = GetTrajectoryTurnAngle();
	return FMath::Abs(TrajectoryTurnAngle) >= AngleThreshold;
}

bool UNinjaGASPAnimInstance::IsPivotingInStateMachine_Implementation() const
{
	const float TurnAngle = GetTrajectoryTurnAngle();

	auto MapRangeClamped = [](const float Value, const float InMin, const float InMax, const float OutMin, const float OutMax)
	{
		return FMath::GetMappedRangeValueClamped(
			FVector2D(InMin, InMax),
			FVector2D(OutMin, OutMax),
			Value);
	};

	const bool bWalkSpeed = Speed2D >= 50.f && Speed2D <= 200.f;
	const bool bRunSpeed = Speed2D >= 200.f && Speed2D <= 550.f;
	const bool bSprintSpeed = Speed2D >= 200.f && Speed2D <= 700.f;
	
	const float WalkAngleThreshold = MapRangeClamped(Speed2D, 150.f, 200.f, 70.f, 60.f);
	const float RunAngleThreshold = MapRangeClamped(Speed2D, 300.f, 500.f, 70.f, 60.f);
	const float SprintAngleThreshold = MapRangeClamped(Speed2D, 300.f, 700.f, 60.f, 50.f);

	float AngleThreshold = 0.f;
	if (Stance == ECharacterStance::Crouch)
	{
		AngleThreshold = 65.f;
	}
	else switch (Gait)
	{
		case ECharacterGait::Walk:
			AngleThreshold = WalkAngleThreshold;
			break;
		case ECharacterGait::Run:
			AngleThreshold = RunAngleThreshold;
			break;
		case ECharacterGait::Sprint:
			AngleThreshold = SprintAngleThreshold;
			break;
		default:
			AngleThreshold = WalkAngleThreshold;
			break;
	}

	bool bSpeedInRangeForGait = false;
	switch (Gait)
	{
		case ECharacterGait::Walk:
			bSpeedInRangeForGait = bWalkSpeed;
			break;
		case ECharacterGait::Run:
			bSpeedInRangeForGait = bRunSpeed;
			break;
		case ECharacterGait::Sprint:
			bSpeedInRangeForGait = bSprintSpeed;
			break;
		default:
			bSpeedInRangeForGait = false;
			break;
	}

	const bool bAngleCondition = FMath::Abs(TurnAngle) >= AngleThreshold;
	const bool bSpeedCondition = Stance == ECharacterStance::Crouch ? bWalkSpeed : bSpeedInRangeForGait;
	
	return bAngleCondition && bSpeedCondition;
}

void UNinjaGASPAnimInstance::SetBlendStackAnimFromChooser_Implementation(const EAnimationStateMachineState NewState, const bool bForceBlend)
{
	StateMachineState = NewState;
	PreviousBlendStackInputs = BlendStackInputs;
	AnimationControlFlags.Reset();

	FAnimationMotionMatchingChooserOutputs AnimationOutputs;
	FChooserEvaluationContext ChooserEvaluationContext = UChooserFunctionLibrary::MakeChooserEvaluationContext();
	ChooserEvaluationContext.AddObjectParam(this);
	ChooserEvaluationContext.AddStructParam(AnimationOutputs);

	const FInstancedStruct ChooserInstance = UChooserFunctionLibrary::MakeEvaluateChooser(StateMachineAnimationTable);
	const TArray<UObject*> ChooserObjects = UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(ChooserEvaluationContext, ChooserInstance, UAnimationAsset::StaticClass());

	TArray<UAnimationAsset*> SelectedAnimations;
	for (UObject* ChooserObject : ChooserObjects)
	{
		UAnimationAsset* ChooserAnimation = Cast<UAnimationAsset>(ChooserObject);
		if (IsValid(ChooserAnimation))
		{
			SelectedAnimations.Add(ChooserAnimation);
		}
	}

	if (SelectedAnimations.IsEmpty())
	{
		AnimationControlFlags.bNoValidAnimation = true;
		return;
	}

	BlendStackInputs.Animation = SelectedAnimations[0];
	BlendStackInputs.StartTime = AnimationOutputs.StartTime;
	BlendStackInputs.BlendTime = AnimationOutputs.BlendTime;
	BlendStackInputs.Tags = AnimationOutputs.Tags;
	BlendStackInputs.BlendProfile = GetBlendProfileByName(AnimationOutputs.BlendProfileName);
	UPoseSearchLibrary::IsAnimationAssetLooping(SelectedAnimations[0], BlendStackInputs.bLoop);
	
	if (AnimationOutputs.bUseMotionMatching)
	{
		FPoseSearchBlueprintResult Result;
		
		static const FName PoseHistoryName = TEXT("PoseHistory");
		const FPoseSearchContinuingProperties ContinuingProperties = FPoseSearchContinuingProperties();
		const FPoseSearchFutureProperties FutureProperties = FPoseSearchFutureProperties();
		UPoseSearchLibrary::MotionMatch(this, ChooserObjects, PoseHistoryName, ContinuingProperties, FutureProperties, Result);

		SearchCost = Result.SearchCost;
		const float CostLimit = AnimationOutputs.MotionMatchingCostLimit;
		const bool bCanAfford = CostLimit > 0.f ? SearchCost <= CostLimit : true;

		const UAnimationAsset* Animation = Cast<UAnimationAsset>(Result.SelectedAnim);
		if (IsValid(Animation) && bCanAfford)
		{
			BlendStackInputs.Animation = Animation;
			BlendStackInputs.StartTime = Result.SelectedTime;
			UPoseSearchLibrary::IsAnimationAssetLooping(SelectedAnimations[0], BlendStackInputs.bLoop);
		}
		else
		{
			AnimationControlFlags.bNoValidAnimation = true;
			return;
		}
	}

	if (bForceBlend)
	{
		FAnimNodeReference Node = GetStateMachineBlendStackNode();
		EAnimNodeReferenceConversionResult BlendStackNodeResult;
		
		FBlendStackAnimNodeReference NodeReference = UBlendStackAnimNodeLibrary::ConvertToBlendStackNode(Node, BlendStackNodeResult);
		if (BlendStackNodeResult == EAnimNodeReferenceConversionResult::Succeeded)
		{
			UBlendStackAnimNodeLibrary::ForceBlendNextUpdate(NodeReference);
		}
	}
}
