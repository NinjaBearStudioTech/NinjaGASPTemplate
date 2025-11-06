// Ninja Bear Studio Inc., all rights reserved.
#include "Animation/NinjaGASPAnimInstance.h"

#include "AnimationWarpingLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"
#include "Interfaces/PlayerCameraModeInterface.h"
#include "Kismet/KismetMathLibrary.h"

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

	bUseVelocityToDetermineMovement = false;
	bUseTrajectoryFutureVelocityToDetermineMovement = true;
	bUseAccelerationToDetermineMovement = true;
	SpeedThreshold = 5.f;
	VelocityAccelerationThreshold = 0.001f;
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
