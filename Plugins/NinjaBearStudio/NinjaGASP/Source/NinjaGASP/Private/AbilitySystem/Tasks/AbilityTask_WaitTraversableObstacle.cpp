// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Tasks/AbilityTask_WaitTraversableObstacle.h"

#include "ChooserFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"
#include "Interfaces/CombatSystemInterface.h"
#include "Interfaces/TraversalActionTransformInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "Types/FCharacterMovementTraversalCheckInputs.h"
#include "Types/FCharacterMovementTraversalChooserTypes.h"

UAbilityTask_WaitTraversableObstacle* UAbilityTask_WaitTraversableObstacle::CreateTask(UGameplayAbility* OwningAbility, const ECollisionChannel TraversalTraceChannel, UChooserTable* TraversalChooserTable, TSubclassOf<UAbilityTask_WaitTraversableObstacle> TaskClass)
{
	TaskClass = IsValid(TaskClass) ? TaskClass.Get() : StaticClass();	
	UAbilityTask_WaitTraversableObstacle* NewTask = NewObject<UAbilityTask_WaitTraversableObstacle>(GetTransientPackage(), TaskClass);

	NewTask->TraversalTraceChannel = TraversalTraceChannel;
	NewTask->ChooserTable = TraversalChooserTable;
	NewTask->InitTask(*OwningAbility, OwningAbility->GetGameplayTaskDefaultPriority());
	
	return NewTask;	
}

void UAbilityTask_WaitTraversableObstacle::Activate()
{
	Super::Activate();
	
	FCharacterMovementTraversalCheckResult Result;

	auto FailAndEnd = [&](const ECharacterTraversalOutcome& Outcome)
	{
		Result.Outcome = Outcome;
		OnTraversableObstacleSearchFinished.Broadcast(Result);
		EndTask();
	};

	// Initialize cached values (capsule, debug CVars, etc.)
	if (!InitializeValues())
	{
		FailAndEnd(ECharacterTraversalOutcome::NotInitialized);
		return;
	}

	// Build traversal scan inputs (forward direction, distance, trace size...)
	const TOptional<FCharacterMovementTraversalCheckInputs> InputsOptional = GetTraversalCheckInputs();
	if (!InputsOptional.IsSet())
	{
		FailAndEnd(ECharacterTraversalOutcome::NotInitialized);
		return;
	}
	
	const FCharacterMovementTraversalCheckInputs& Inputs = InputsOptional.GetValue();

	// Forward sweep to find a traversable obstacle.
	FHitResult TraversalHit;
	UPrimitiveComponent* TraversableObstacle = ScanForPrimitiveComponent(Inputs, TraversalHit);
	if (!IsValid(TraversableObstacle))
	{
		FailAndEnd(ECharacterTraversalOutcome::TraversalNotFound);
		return;
	}

	Result.HitComponent = TraversableObstacle;

	// Compute front/back ledges and their normals from the initial hit.
	TryAndCalculateLedges(TraversalHit, Result);
	if (!Result.bHasFrontLedge)
	{
		FailAndEnd(ECharacterTraversalOutcome::TraversalNotFound);
		return;
	}

	// Check there is enough room above the obstacle for our capsule.
	if (!CheckSpaceAboveObstacle(Result))
	{
		FailAndEnd(ECharacterTraversalOutcome::NotEnoughRoomOnTop);
		return;
	}

	// Sweep across the top of the obstacle and check how much depth we have,
	// plus whether there's a back ledge and floor behind the obstacle.
	PerformTopSweepAcrossObstacle(Result);
	CheckFloorBehindObstacle(Result);

	// Pick an animation via chooser + motion matching.
	if (!ChooseAnimationForTraversalAction(Result))
	{
		FailAndEnd(ECharacterTraversalOutcome::MontageSelectionFailed);
		return;
	}

	// Success!
	Result.Outcome = ECharacterTraversalOutcome::TraversalFound;
	OnTraversableObstacleSearchFinished.Broadcast(Result);
	EndTask();
}

void UAbilityTask_WaitTraversableObstacle::SetAnimInstance(UAnimInstance* NewAnimInstance)
{
	if (IsValid(NewAnimInstance))
	{
		AnimInstance = NewAnimInstance;	
	}
}

bool UAbilityTask_WaitTraversableObstacle::InitializeValues()
{
	CharacterAvatar = Cast<ACharacter>(GetAvatarActor());
	if (!IsValid(CharacterAvatar))
	{
		return false;
	}

	const UCapsuleComponent* Capsule = CharacterAvatar->GetCapsuleComponent();
	if (!IsValid(Capsule))
	{
		return false;
	}

	ActorLocation = CharacterAvatar->GetActorLocation();
	CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	
	static const TCHAR* DebugLevelVariable = TEXT("DDCvar.Traversal.DrawDebugLevel"); 
	DrawDebugLevel = UKismetSystemLibrary::GetConsoleVariableIntValue(DebugLevelVariable);

	static const TCHAR* DebugDurationVariable = TEXT("DDCvar.Traversal.DrawDebugDuration"); 
	DrawDebugDuration = UKismetSystemLibrary::GetConsoleVariableFloatValue(DebugDurationVariable);
	
	return true;
}

TOptional<FCharacterMovementTraversalCheckInputs> UAbilityTask_WaitTraversableObstacle::GetTraversalCheckInputs() const
{
    if (!IsValid(CharacterAvatar))
    {
        return TOptional<FCharacterMovementTraversalCheckInputs>();
    }

    const UCharacterMovementComponent* CMC = CharacterAvatar->GetCharacterMovement();
    if (!IsValid(CMC))
    {
    	return TOptional<FCharacterMovementTraversalCheckInputs>();
    }

    const FVector ForwardVector = CharacterAvatar->GetActorForwardVector();
    auto MakeInputs = [&](const float Distance, const float HalfHeight, const FVector& OriginOffset, const FVector& EndOffset)
    {
        FCharacterMovementTraversalCheckInputs Inputs;
        Inputs.TraceForwardDirection = ForwardVector;
        Inputs.TraceForwardDistance = Distance;
        Inputs.TraceOriginOffset = OriginOffset;
        Inputs.TraceEndOffset = EndOffset;
        Inputs.TraceRadius = 30.f;
        Inputs.TraceHalfHeight = HalfHeight;
        return TOptional<FCharacterMovementTraversalCheckInputs>(Inputs);
    };

    switch (CMC->MovementMode)
    {
        case MOVE_Falling:
        case MOVE_Flying:
        {
            constexpr float Distance   = 75.f;
            constexpr float HalfHeight = 86.f;
            const FVector OriginOffset = FVector::ZeroVector;
            const FVector EndOffset(0.f, 0.f, 50.f);
            return MakeInputs(Distance, HalfHeight, OriginOffset, EndOffset);
        }

        default:
        case MOVE_None:
        case MOVE_Walking:
        case MOVE_NavWalking:
        case MOVE_Swimming:
        case MOVE_Custom:
        {
            const FVector Velocity = CMC->Velocity;
            const FRotator Rotation = CharacterAvatar->GetActorRotation();
            const FVector Unrotated = Rotation.UnrotateVector(Velocity);
            const float Distance = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 500.f), FVector2D(75.f, 350.f), Unrotated.X);

            constexpr float HalfHeight = 60.f;
            const FVector OriginOffset = FVector::ZeroVector;
            const FVector EndOffset = FVector::ZeroVector;

            return MakeInputs(Distance, HalfHeight, OriginOffset, EndOffset);
        }
    }
}

UPrimitiveComponent* UAbilityTask_WaitTraversableObstacle::ScanForPrimitiveComponent(const FCharacterMovementTraversalCheckInputs& Inputs, FHitResult& TraversalHit)
{
	if (!IsValid(CharacterAvatar))
	{
		return nullptr;
	}

	const UWorld* World = CharacterAvatar->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector TraceStart = ActorLocation + Inputs.TraceOriginOffset;
	const FVector TraceEnd = TraceStart + (Inputs.TraceForwardDirection * Inputs.TraceForwardDistance) + Inputs.TraceEndOffset;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(TraversalCapsuleTrace));
	Params.bTraceComplex = false;
	Params.AddIgnoredActor(CharacterAvatar);

	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(Inputs.TraceRadius, Inputs.TraceHalfHeight);

	FHitResult Hit;
	const bool bResult = World->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, TraversalTraceChannel, CapsuleShape, Params);

#if ENABLE_DRAW_DEBUG
	if (DrawDebugLevel >= 2)
	{
		DrawDebugCapsule(World, TraceStart, Inputs.TraceHalfHeight, Inputs.TraceRadius, FQuat::Identity, FColor::Green, false, DrawDebugDuration);
		DrawDebugCapsule(World, TraceEnd, Inputs.TraceHalfHeight, Inputs.TraceRadius, FQuat::Identity, bResult ? FColor::Red : FColor::Green, false, DrawDebugDuration);
	}
#endif

	if (!bResult)
	{
		return nullptr;
	}

	TraversalHit = Hit;
	return Hit.GetComponent();
}

void UAbilityTask_WaitTraversableObstacle::TryAndCalculateLedges(FHitResult& OriginalHit, FCharacterMovementTraversalCheckResult& InOutResult)
{
	const FVector Direction = CharacterAvatar->GetActorForwardVector();

	bool bFoundFrontLedge = false;
	bool bFoundBackLedge = false;
	FVector StartLedgeLocation = FVector::ZeroVector;
	FVector StartLedgeNormal = FVector::ZeroVector;
	FVector EndLedgeLocation = FVector::ZeroVector;
	FVector EndLedgeNormal = FVector::ZeroVector;
	
	ComputeLedgeData(OriginalHit, Direction, bFoundFrontLedge, bFoundBackLedge, StartLedgeLocation, StartLedgeNormal, EndLedgeLocation, EndLedgeNormal);

	InOutResult.bHasFrontLedge = bFoundFrontLedge;
	InOutResult.bHasBackLedge = bFoundBackLedge;
	InOutResult.FrontLedgeLocation = StartLedgeLocation;
	InOutResult.FrontLedgeNormal = StartLedgeNormal;
	InOutResult.BackLedgeLocation = EndLedgeLocation;
	InOutResult.BackLedgeNormal = EndLedgeNormal;
}

bool UAbilityTask_WaitTraversableObstacle::CheckSpaceAboveObstacle(FCharacterMovementTraversalCheckResult& InOutResult)
{
    AActor* Avatar = Cast<AActor>(GetAvatarActor());
    if (!Avatar)
    {
        return false;
    }

    UWorld* World = Avatar->GetWorld();
    if (!World)
    {
        return false;
    }

    const FVector TraceStart = ActorLocation;

    const FVector ExtraSpace(0.f, 0.f, CapsuleHalfHeight + 2.f);
    FrontLedgeLocationCheck = InOutResult.FrontLedgeLocation + (InOutResult.FrontLedgeNormal * (CapsuleRadius + 2.f)) + ExtraSpace;

    const FVector TraceEnd = FrontLedgeLocationCheck;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(CheckSpaceAboveObstacle));
    Params.bTraceComplex = false;
	Params.AddIgnoredActor(Avatar);

    const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

    FHitResult Hit;
    World->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, CapsuleShape, Params);
    const bool bHasRoom = !(Hit.bBlockingHit || Hit.bStartPenetrating);

#if ENABLE_DRAW_DEBUG
    if (DrawDebugLevel >= 3)
    {
        static constexpr bool  bPersistentLines = false;
        static const float LifeTime = DrawDebugDuration;

        const FColor TraceColor = FColor::Red;
        const FColor HitColor = FColor::Green;

        DrawDebugCapsule(World, TraceStart, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, TraceColor, bPersistentLines, LifeTime );
        DrawDebugCapsule(World, TraceEnd, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, bHasRoom ? HitColor : TraceColor, bPersistentLines, LifeTime );
        DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, bPersistentLines, LifeTime);
    }
#endif

    if (bHasRoom)
    {
        const FVector TotalHeight = ActorLocation - FVector(0.f, 0.f, CapsuleHalfHeight) - InOutResult.FrontLedgeLocation;
        InOutResult.ObstacleHeight = FMath::Abs(TotalHeight.Z);
    }
    else
    {
    	InOutResult.bHasFrontLedge = false;
    }

    return bHasRoom;
}

bool UAbilityTask_WaitTraversableObstacle::PerformTopSweepAcrossObstacle(FCharacterMovementTraversalCheckResult& InOutResult)
{
    AActor* Avatar = Cast<AActor>(GetAvatarActor());
    if (!Avatar)
    {
        return false;
    }

    UWorld* World = Avatar->GetWorld();
    if (!World)
    {
        return false;
    }

    const float  NormalOffset = CapsuleRadius + 2.f;
    const FVector ExtraSpace(0.f, 0.f, CapsuleHalfHeight + 2.f);

    BackLedgeLocationCheck = InOutResult.BackLedgeLocation + (InOutResult.BackLedgeNormal * NormalOffset) + ExtraSpace;
    const FVector TraceStart = FrontLedgeLocationCheck;
    const FVector TraceEnd = BackLedgeLocationCheck;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(CheckSpaceAcrossObstacle));
    Params.bTraceComplex = false;
	Params.AddIgnoredActor(Avatar);

    const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

    FHitResult Hit;
    const bool bHit = World->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, CapsuleShape, Params);

    if (bHit)
    {
        // No room to fully cross. Obstacle depth is from front ledge to impact point.
        InOutResult.bHasBackLedge = false;
        InOutResult.ObstacleDepth = FVector::Dist2D(InOutResult.FrontLedgeLocation, Hit.ImpactPoint);
    }
    else
    {
        // There is room. Depth is distance between front and back ledges.
        InOutResult.bHasBackLedge = true;
        InOutResult.ObstacleDepth = FVector::Dist2D(InOutResult.FrontLedgeLocation, InOutResult.BackLedgeLocation);
    }

#if ENABLE_DRAW_DEBUG
    if (DrawDebugLevel >= 3)
    {
        static constexpr bool bPersistentLines = false;
        static const float LifeTime = DrawDebugDuration;

        const FColor TraceColor = FColor::Red;
        const FColor HitColor = FColor::Green;

        DrawDebugCapsule(World, TraceStart, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, TraceColor, bPersistentLines, LifeTime);
        DrawDebugCapsule(World, TraceEnd, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, bHit ? TraceColor : HitColor, bPersistentLines, LifeTime);
        DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, bPersistentLines, LifeTime);
    }
#endif

    return !bHit;
}

bool UAbilityTask_WaitTraversableObstacle::CheckFloorBehindObstacle(FCharacterMovementTraversalCheckResult& InOutResult)
{
    AActor* Avatar = Cast<AActor>(GetAvatarActor());
    if (!Avatar)
    {
        InOutResult.bHasBackFloor = false;
        return false;
    }

    UWorld* World = Avatar->GetWorld();
    if (!World)
    {
        InOutResult.bHasBackFloor = false;
        return false;
    }

    const float DownDistance = InOutResult.ObstacleHeight + 50.f;
	const FVector TraceStart = BackLedgeLocationCheck;
    const FVector TraceEnd = TraceStart - FVector(0.f, 0.f, DownDistance);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(CheckFloorBehindObstacle));
    Params.bTraceComplex = false;
	Params.AddIgnoredActor(Avatar);

    const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

    FHitResult Hit;
    const bool bHit = World->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, CapsuleShape, Params);

#if ENABLE_DRAW_DEBUG
    if (DrawDebugLevel >= 3)
    {
        static constexpr bool bPersistentLines = false;
        static const float LifeTime = DrawDebugDuration;

        const FColor TraceColor = FColor::Red;
        const FColor HitColor = FColor::Green;

        DrawDebugCapsule(World, TraceStart, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, TraceColor, bPersistentLines, LifeTime);
        DrawDebugCapsule(World, TraceEnd, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, bHit ? HitColor : TraceColor, bPersistentLines, LifeTime);
        DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, bPersistentLines, LifeTime);
    }
#endif

    if (!bHit || !Hit.bBlockingHit)
    {
        InOutResult.bHasBackFloor = false;
        return false;
    }

    InOutResult.bHasBackFloor = true;
    InOutResult.BackFloorLocation = Hit.ImpactPoint;

    const FVector HeightDelta = Hit.ImpactPoint - InOutResult.BackLedgeLocation;
    InOutResult.BackLedgeHeight = FMath::Abs(HeightDelta.Z);

    return true;
}

bool UAbilityTask_WaitTraversableObstacle::ChooseAnimationForTraversalAction(FCharacterMovementTraversalCheckResult& InOutResult)
{
	if (!IsValid(CharacterAvatar))
	{
		return false;
	}

	UCharacterMovementComponent* CMC = CharacterAvatar->GetCharacterMovement();
	if (!IsValid(CMC))
	{
		return false;
	}
	
	ECharacterGait Gait = ECharacterGait::Run;
	if (CharacterAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		Gait = IAdvancedCharacterMovementInterface::Execute_GetCharacterGait(CharacterAvatar);
	}
	
	FCharacterMovementTraversalChooserInput Input;
	Input.Action = InOutResult.Action;
	Input.bHasFrontLedge = InOutResult.bHasFrontLedge;
	Input.bHasBackLedge = InOutResult.bHasBackLedge;
	Input.bHasBackFloor = InOutResult.bHasBackFloor;
	Input.ObstacleHeight = InOutResult.ObstacleHeight;
	Input.ObstacleDepth = InOutResult.ObstacleDepth;
	Input.BackLedgeHeight = InOutResult.BackLedgeHeight;
	Input.MovementMode = CMC->MovementMode;
	Input.Gait = Gait;
	Input.Speed = CMC->Velocity.Size2D();
	
	FCharacterMovementTraversalChooserOutput Output;
	
	FChooserEvaluationContext ChooserEvaluationContext = UChooserFunctionLibrary::MakeChooserEvaluationContext();
	ChooserEvaluationContext.AddStructParam(Input);
	ChooserEvaluationContext.AddStructParam(Output);
	
	const FInstancedStruct ChooserInstance = UChooserFunctionLibrary::MakeEvaluateChooser(ChooserTable);
	const TArray<UObject*> ChooserObjects = UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(ChooserEvaluationContext, ChooserInstance, UAnimationAsset::StaticClass());

	InOutResult.Action = Output.Action;

	if (DrawDebugLevel >= 1)
	{
		const FString Conditions =
			FString::Printf(TEXT("Traversal:\n")
				TEXT("  HasFrontLedge: %s\n")
				TEXT("  HasBackLedge:  %s\n")
				TEXT("  HasBackFloor:  %s\n")
				TEXT("  ObstacleHeight: %.2f\n")
				TEXT("  ObstacleDepth:  %.2f\n")
				TEXT("  BackLedgeHeight: %.2f"),
				InOutResult.bHasFrontLedge ? TEXT("true") : TEXT("false"),
				InOutResult.bHasBackLedge ? TEXT("true") : TEXT("false"),
				InOutResult.bHasBackFloor ? TEXT("true") : TEXT("false"),
				InOutResult.ObstacleHeight,
				InOutResult.ObstacleDepth,
				InOutResult.BackLedgeHeight
			);

		if (GEngine)
		{
			const UEnum* ActionEnum = StaticEnum<ECharacterTraversalAction>();
			const FString ActionStr = ActionEnum ? ActionEnum->GetNameStringByValue(static_cast<int64>(InOutResult.Action)) : TEXT("Unknown");
			GEngine->AddOnScreenDebugMessage(-1,2.0f, FColor::Cyan, Conditions);
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta, FString::Printf(TEXT("Traversal Action: %s"), *ActionStr));
		}
	}
	
	if (InOutResult.Action == ECharacterTraversalAction::None)
	{
		return false;	
	}

	if (!IsValid(AnimInstance))
	{
		if (CharacterAvatar->Implements<UCombatSystemInterface>())
		{
			AnimInstance = ICombatSystemInterface::Execute_GetCombatAnimInstance(CharacterAvatar);	
		}
		else
		{
			AnimInstance = CharacterAvatar->GetMesh()->GetAnimInstance();
		}
	}
	if (!IsValid(AnimInstance) || !AnimInstance->Implements<UTraversalActionTransformInterface>())
	{
		return false;
	}

	FTransform TraversalTransform = FTransform();
	TraversalTransform.SetLocation(InOutResult.FrontLedgeLocation);
	TraversalTransform.SetRotation(FRotationMatrix::MakeFromZ(InOutResult.FrontLedgeNormal).ToQuat());
	TraversalTransform.SetScale3D(FVector::OneVector);
	ITraversalActionTransformInterface::Execute_SetTraversalTransform(AnimInstance, TraversalTransform);

	FPoseSearchBlueprintResult Result;
	const FName PoseHistoryName = TEXT("PoseHistory");
	UPoseSearchLibrary::MotionMatch(AnimInstance, ChooserObjects, PoseHistoryName, FPoseSearchContinuingProperties(), FPoseSearchFutureProperties(), Result);

	UAnimMontage* MontageToPlay = Cast<UAnimMontage>(Result.SelectedAnim);
	if (!IsValid(MontageToPlay))
	{
		return false;
	}

	InOutResult.MontageToPlay = MontageToPlay;
	InOutResult.StartTime = Result.SelectedTime;
	InOutResult.PlayRate = Result.WantedPlayRate;
	return true;
}

void UAbilityTask_WaitTraversableObstacle::ComputeLedgeData(FHitResult& Hit, const FVector& Direction, bool& bFoundFrontLedge,
	bool& bFoundBackLedge, FVector& StartLedgeLocation, FVector& StartLedgeNormal, FVector& EndLedgeLocation, FVector& EndLedgeNormal) const
{
	auto ZeroOutAndFail = [&]()
	{
		bFoundFrontLedge = false;
		bFoundBackLedge = false;
		StartLedgeLocation = FVector::ZeroVector;
		StartLedgeNormal = FVector::ZeroVector;
		EndLedgeLocation = FVector::ZeroVector;
		EndLedgeNormal = FVector::ZeroVector;
	};

	UPrimitiveComponent* HitComp = Hit.GetComponent();
	if (!IsValid(HitComp) || !IsValid(CharacterAvatar))
	{
		ZeroOutAndFail();
		return;
	}

	// Make sure our initial hit is nudged slightly towards the object's origin.
	// This helps when the initial trace hits right at the edge of the object.
	NudgeTraceTowardsObjectOrigin(Hit);

	const FVector StartNormal = Hit.ImpactNormal;
	FVector Origin, BoxExtent;
	float SphereRadius = 0.f;
	UKismetSystemLibrary::GetComponentBounds(HitComp, Origin, BoxExtent, SphereRadius);
	const float TraceLength = SphereRadius * 2.f;

	// To make sure we can mantle objects that are upside down, we make sure the
	// object's "up" vector always points roughly in the same direction as the player's up.
	const FVector ComponentUp = HitComp->GetUpVector();
	const FVector ActorUp = CharacterAvatar->GetActorUpVector();
	const float UpDot = FVector::DotProduct(ComponentUp, ActorUp);
	const float UpSign = UKismetMathLibrary::SignOfFloat(UpDot);
	const FVector AbsoluteObjectUpVector = ComponentUp * UpSign;

	// ----
	// Offset the trace towards the opposite direction of corners, by the half ledge width,
	// ensuring we aren't grabbing air if we are close to a corner.
	// ----
	
	bool bCloseToCorner = false;
	float DistanceToCorner = 0.f;
	FVector OffsetCornerPoint = FVector::ZeroVector;

	FVector TraceDirection = FVector::CrossProduct(Hit.ImpactNormal, AbsoluteObjectUpVector);
	TraceCorners(Hit, TraceDirection, TraceLength, bCloseToCorner, DistanceToCorner, OffsetCornerPoint);

	const float RightEdgeDistance = DistanceToCorner;
	if (bCloseToCorner)
	{
		Hit.ImpactPoint = OffsetCornerPoint;
	}

	TraceDirection = FVector::CrossProduct(AbsoluteObjectUpVector, Hit.ImpactNormal);
	TraceCorners(Hit, TraceDirection, TraceLength, bCloseToCorner, DistanceToCorner, OffsetCornerPoint);

	if (bCloseToCorner && (DistanceToCorner + RightEdgeDistance < MinLedgeWidth))
	{
		Hit.ImpactPoint = OffsetCornerPoint;
	}

	// ----
	// If the corners of the current hit plane are too close together, it doesn't necessarily mean
	// we can't traverse this object (e.g. a cylinder). Try to see if we'll hit the object with our
	// width params left and right.
	// ----
	
	FHitResult TraceWidthHit;

	TraceDirection = FVector::CrossProduct(AbsoluteObjectUpVector, Hit.ImpactNormal);
	if (!TraceWidth(Hit, TraceDirection, TraceWidthHit))
	{
		TraceDirection *= -1.f;

		if (!TraceWidth(Hit, TraceDirection, TraceWidthHit))
		{
			ZeroOutAndFail();
			return;
		}
	}

	// ----
	// Given our (possibly adjusted) hit, check for a front edge by
	// tracing towards the hit point along the plane.
	// ----
	
	FVector StartLedge = FVector::ZeroVector;
	FHitResult TraceAlongPlaneStartHit;

	if (TraceAlongHitPlane(Hit, AbsoluteObjectUpVector, TraceLength, TraceAlongPlaneStartHit))
	{
		StartLedge = TraceAlongPlaneStartHit.ImpactPoint;

#if ENABLE_DRAW_DEBUG
		if (bPersistentShowTrace && DrawDebugLevel >= 3)
		{
			static constexpr float VeryLongDuration = 100000000.f;
			DrawDebugSphere(CharacterAvatar->GetWorld(), StartLedge, 10.f, 12, FColor::Green, false, VeryLongDuration);
		}
#endif
	}
	else
	{
		ZeroOutAndFail();
		return;
	}

	// ----
	// Reverse our trace to figure out the penetrating point of the backface of the object,
	// and then try to find the back ledge by tracing along that backface plane from above.
	// ----
	
	FVector EndNormal = FVector::ZeroVector;
	FVector EndLedge = FVector::ZeroVector;

	const FVector ComponentTraceStart = Hit.ImpactPoint - Hit.ImpactNormal * TraceLength;

	FHitResult ComponentLineTraceHit;
	FCollisionQueryParams BackFaceParams(SCENE_QUERY_STAT(TraversalBackFaceTrace));
	BackFaceParams.bTraceComplex = true;
	BackFaceParams.AddIgnoredActor(CharacterAvatar);

	const bool bHasBackLedge = HitComp->LineTraceComponent(ComponentLineTraceHit, ComponentTraceStart, Hit.ImpactPoint, BackFaceParams);

	if (bHasBackLedge)
	{
		EndNormal = ComponentLineTraceHit.ImpactNormal;

		FHitResult TraceAlongPlaneEndHit;
		if (TraceAlongHitPlane(ComponentLineTraceHit, AbsoluteObjectUpVector, TraceLength, TraceAlongPlaneEndHit))
		{
			EndLedge = TraceAlongPlaneEndHit.ImpactPoint;

#if ENABLE_DRAW_DEBUG
			if (bPersistentShowTrace && DrawDebugLevel >= 3)
			{
				static constexpr float VeryLongDuration = 100000000.f;
				DrawDebugSphere(CharacterAvatar->GetWorld(), EndLedge, 10.f, 12, FColor::Cyan, false, VeryLongDuration);
			}
#endif
		}
	}

	// ----
	// Populate outputs. If we made it here, we always consider the front ledge found.
	// The back ledge is only valid if the backface trace hit.
	// ----
	
	bFoundFrontLedge = true;
	bFoundBackLedge = bHasBackLedge;
	StartLedgeLocation = StartLedge;
	StartLedgeNormal = StartNormal;
	EndLedgeLocation = EndLedge;
	EndLedgeNormal = EndNormal;
}

void UAbilityTask_WaitTraversableObstacle::NudgeTraceTowardsObjectOrigin(FHitResult& Hit) const
{
	const UPrimitiveComponent* PrimitiveComponentHit = Hit.GetComponent();
	if (!IsValid(PrimitiveComponentHit))
	{
		return;
	}

	FVector Origin, BoxExtent; float SphereRadius;
	UKismetSystemLibrary::GetComponentBounds(PrimitiveComponentHit, Origin, BoxExtent, SphereRadius);

	const FVector Intersection = UKismetMathLibrary::ProjectPointOnToPlane(Origin, Hit.ImpactPoint, Hit.ImpactNormal);
	const FVector DirectionUnit = UKismetMathLibrary::GetDirectionUnitVector( Intersection, Hit.ImpactPoint);
	const FVector NewImpactPoint = Hit.ImpactPoint - DirectionUnit;

	Hit.ImpactPoint= NewImpactPoint;
	Hit.Location = NewImpactPoint;
}

void UAbilityTask_WaitTraversableObstacle::TraceCorners(const FHitResult& Hit, const FVector& TraceDirection,
	const float TraceLength, bool& bCloseToCorner, float& DistanceToCorner, FVector& OffsetCornerPoint) const
{
	FHitResult PlaneHit;
	const bool bResult = TraceAlongHitPlane(Hit, TraceDirection, TraceLength, PlaneHit);

	if (!bResult)
	{
		bCloseToCorner = false;
		DistanceToCorner = 0.0f;
		OffsetCornerPoint = FVector::ZeroVector;
		return;
	}

	const float HalfMinLedgeWidth = MinLedgeWidth / 2.f;
	DistanceToCorner = FVector::Distance(PlaneHit.ImpactPoint, Hit.ImpactPoint);
	bCloseToCorner = DistanceToCorner < HalfMinLedgeWidth;
	OffsetCornerPoint = PlaneHit.ImpactPoint - TraceDirection * HalfMinLedgeWidth;
}

bool UAbilityTask_WaitTraversableObstacle::TraceWidth(const FHitResult& Hit, const FVector& Direction, FHitResult& OutHit) const
{
    UPrimitiveComponent* HitComp = Hit.GetComponent();
    if (!IsValid(HitComp))
    {
        return false;
    }

    const FVector DirOffset = Direction * (MinLedgeWidth * 0.5f);
    const FVector DepthOffset = Hit.ImpactNormal * MinFrontLedgeDepth;

    const FVector TraceStart = Hit.ImpactPoint + DirOffset + DepthOffset;
    const FVector TraceEnd = Hit.ImpactPoint + DirOffset - DepthOffset;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(TraversalTraceWidth));
	Params.bTraceComplex = true;
	Params.AddIgnoredActor(CharacterAvatar);
    const bool bHit = HitComp->LineTraceComponent(OutHit, TraceStart, TraceEnd, Params);

#if ENABLE_DRAW_DEBUG
    if (DrawDebugLevel >= 3)
    {
        if (const UWorld* World = GetWorld())
        {
            const FColor LineColor = bHit ? FColor::Red : FColor::Green;
            const FColor ImpactColor = bHit ? FColor::Yellow : FColor::Transparent;
            const float Duration = DrawDebugDuration > 0.f ? DrawDebugDuration : 2.f;

            DrawDebugLine(World, TraceStart, TraceEnd, LineColor, false, Duration, 0, 1.5f);
            if (bHit)
            {
                DrawDebugPoint(World, OutHit.ImpactPoint, 12.f, ImpactColor, false, Duration);
                DrawDebugDirectionalArrow(World, OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal * 20.f, 6.f, ImpactColor, false, Duration, 0, 1.0f);
            }
        }
    }
#endif

    return bHit;
}

bool UAbilityTask_WaitTraversableObstacle::TraceAlongHitPlane(const FHitResult& Hit, const FVector& TraceDirection, const float TraceLength, FHitResult& OutHit) const
{
    UPrimitiveComponent* HitComp = Hit.GetComponent();
    if (!IsValid(HitComp))
    {
        return false;
    }

    const FVector RightVectorAlignedWithRotation = Hit.ImpactNormal.Cross(TraceDirection);
    FVector UpVectorAlongSurface = RightVectorAlignedWithRotation.Cross(Hit.ImpactNormal);
    UpVectorAlongSurface.Normalize();

    const FVector OriginalHitLocation = Hit.ImpactPoint - Hit.ImpactNormal;
    const FVector NewTraceStart = OriginalHitLocation + UpVectorAlongSurface * TraceLength;

    const FVector NewTraceEnd = UKismetMathLibrary::VLerp(NewTraceStart, OriginalHitLocation, 1.5f);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TraversalTraceAlongHitPlane));
	Params.bTraceComplex = true;
	Params.AddIgnoredActor(CharacterAvatar);

    const bool bHit = HitComp->LineTraceComponent(OutHit, NewTraceStart, NewTraceEnd, Params);

#if ENABLE_DRAW_DEBUG
    if (DrawDebugLevel >= 3)
    {
        if (UWorld* World = GetWorld())
        {
            const float Duration  = DrawDebugDuration > 0.f ? DrawDebugDuration : 2.f;
            const FColor LineColor   = bHit ? FColor::Red   : FColor::Green;
            const FColor ImpactColor = bHit ? FColor::Yellow : FColor::Transparent;

            DrawDebugLine(World, NewTraceStart, NewTraceEnd, LineColor, false, Duration, 0, 1.5f);
            DrawDebugPoint(World, Hit.ImpactPoint, 8.f, FColor::Cyan, false, Duration);
            DrawDebugDirectionalArrow(World, Hit.ImpactPoint, Hit.ImpactPoint + Hit.ImpactNormal * 20.f, 6.f, FColor::Blue, false, Duration, 0, 1.f);

            if (bHit)
            {
                DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, ImpactColor, false, Duration);
                DrawDebugDirectionalArrow(World, OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal * 20.f, 6.f, ImpactColor, false, Duration, 0, 1.f);
            }
        }
    }
#endif

    return bHit;
}
