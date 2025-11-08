// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Tasks/AbilityTask_WaitTraversableObstacle.h"

#include "ChooserFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
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

UAbilityTask_WaitTraversableObstacle* UAbilityTask_WaitTraversableObstacle::CreateTask(UGameplayAbility* OwningAbility, const ETraceTypeQuery TraversalTraceChannel, UChooserTable* TraversalChooserTable)
{
	UAbilityTask_WaitTraversableObstacle* NewTask = NewObject<UAbilityTask_WaitTraversableObstacle>(GetTransientPackage());

	NewTask->TraversalTraceChannel = TraversalTraceChannel;
	NewTask->ChooserTable = TraversalChooserTable;
	NewTask->InitTask(*OwningAbility, OwningAbility->GetGameplayTaskDefaultPriority());
	
	return NewTask;	
}

void UAbilityTask_WaitTraversableObstacle::Activate()
{
	Super::Activate();

	FCharacterMovementTraversalCheckResult Result;
	
	const TOptional<FCharacterMovementTraversalCheckInputs> Inputs = GetTraversalCheckInputs();
	if (!Inputs.IsSet())
	{
		Result.Outcome = ECharacterTraversalOutcome::NotInitialized;
		TraversableObstacleSearchFinished.Broadcast(Result);
		EndTask();
		return;
	}
	
	if (!InitializeValues())
	{
		Result.Outcome = ECharacterTraversalOutcome::NotInitialized;
		TraversableObstacleSearchFinished.Broadcast(Result);
		EndTask();
		return;
	}

	UPrimitiveComponent* TraversableObstacle = ScanForPrimitiveComponent(Inputs.GetValue());
	if (!IsValid(TraversableObstacle))
	{
		Result.Outcome = ECharacterTraversalOutcome::TraversalNotFound;
		TraversableObstacleSearchFinished.Broadcast(Result);
		EndTask();
		return;
	}

	Result.HitComponent = TraversableObstacle;
	CalculateLedges(Inputs.GetValue(), Result);
	if (!Result.bHasFrontLedge)
	{
		Result.Outcome = ECharacterTraversalOutcome::TraversalNotFound;
		TraversableObstacleSearchFinished.Broadcast(Result);
		EndTask();
		return;
	}

	if (!CheckSpaceAboveObstacle(Result))
	{
		Result.Outcome = ECharacterTraversalOutcome::NotEnoughRoomOnTop;
		TraversableObstacleSearchFinished.Broadcast(Result);
		EndTask();
		return;
	}

	CheckSpaceAcrossObstacle(Result);
	CheckFloorBehindObstacle(Result);

	if (!ChooseAnimationForTraversalAction(Result))
	{
		Result.Outcome = ECharacterTraversalOutcome::MontageSelectionFailed;
		TraversableObstacleSearchFinished.Broadcast(Result);
		EndTask();
		return;
	}

	Result.Outcome = ECharacterTraversalOutcome::TraversalFound;
	TraversableObstacleSearchFinished.Broadcast(Result);
}

void UAbilityTask_WaitTraversableObstacle::SetAnimInstance(UAnimInstance* NewAnimInstance)
{
	AnimInstance = NewAnimInstance;
}

void UAbilityTask_WaitTraversableObstacle::SetDebugType(const EDrawDebugTrace::Type NewDebugTrace)
{
	DebugTrace = NewDebugTrace;
}

bool UAbilityTask_WaitTraversableObstacle::InitializeValues()
{
	CharacterAvatar = Cast<ACharacter>(GetAvatarActor());
	if (!IsValid(CharacterAvatar))
	{
		// We need a valid character for this!
		return false;
	}

	static const FString DebugLevelVariable = TEXT("DDCvar.Traversal.DrawDebugLevel"); 
	DrawDebugLevel = UKismetSystemLibrary::GetConsoleVariableIntValue(DebugLevelVariable);

	static const FString DebugDurationVariable = TEXT("DDCvar.Traversal.DrawDebugDuration"); 
	DrawDebugDuration = UKismetSystemLibrary::GetConsoleVariableFloatValue(DebugDurationVariable);
	
	ActorLocation = GetAvatarActor()->GetActorLocation();
	CapsuleRadius =	CharacterAvatar->GetCapsuleComponent()->GetScaledCapsuleRadius();
	CapsuleHalfHeight =	CharacterAvatar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
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
	const FVector Velocity = CMC->Velocity;
	const FRotator ActorRotation = CharacterAvatar->GetActorRotation();

	const FVector Unrotated = ActorRotation.UnrotateVector(Velocity);
	const float Distance = UKismetMathLibrary::MapRangeClamped(Unrotated.X, 0.f, 500.f, 75.f, 350.f);

	switch (CMC->MovementMode)
	{
		default:
		case MOVE_None:
		case MOVE_Walking:
		case MOVE_NavWalking:
		case MOVE_Swimming:
		case MOVE_Custom:
		{
			FCharacterMovementTraversalCheckInputs Inputs;
			Inputs.TraceForwardDirection = ForwardVector;
			Inputs.TraceForwardDistance = Distance;
			Inputs.TraceOriginOffset = FVector::ZeroVector;
			Inputs.TraceEndOffset = FVector::ZeroVector;
			Inputs.TraceRadius = 30.f;
			Inputs.TraceHalfHeight = 60.f;
			return TOptional<FCharacterMovementTraversalCheckInputs>(Inputs); 
		}
		case MOVE_Falling:
		case MOVE_Flying:
		{
			FCharacterMovementTraversalCheckInputs Inputs;
			Inputs.TraceForwardDirection = ForwardVector;
			Inputs.TraceForwardDistance = 75.f;
			Inputs.TraceOriginOffset = FVector::ZeroVector;
			Inputs.TraceEndOffset = FVector::ZeroVector;
			Inputs.TraceRadius = 30.f;
			Inputs.TraceHalfHeight = 66.f;
			return TOptional<FCharacterMovementTraversalCheckInputs>(Inputs); 
		}		
	}	
}

UPrimitiveComponent* UAbilityTask_WaitTraversableObstacle::ScanForPrimitiveComponent(const FCharacterMovementTraversalCheckInputs& Inputs) const
{
	AActor* Context = GetAvatarActor();
	const TArray<AActor*> ActorsToIgnore = { Context };
	
	const FVector TraceStart = ActorLocation + Inputs.TraceOriginOffset;
	const FVector TraceEnd = TraceStart + (Inputs.TraceForwardDirection * Inputs.TraceForwardDistance) + Inputs.TraceEndOffset;
	const float TraceRadius = Inputs.TraceRadius;
	const float TraceHalfHeight = Inputs.TraceHalfHeight;

	const EDrawDebugTrace::Type DebugType = DrawDebugLevel >= 2 ? DebugTrace : EDrawDebugTrace::None;
	static constexpr bool bTraceComplex = false;
	static constexpr bool bIgnoreSelf = true;
	
	FHitResult Hit;
	const bool bResult = UKismetSystemLibrary::CapsuleTraceSingle(Context, TraceStart, TraceEnd, TraceRadius, TraceHalfHeight, TraversalTraceChannel,
		bTraceComplex, ActorsToIgnore, DebugType, Hit, bIgnoreSelf,
		FLinearColor::Black, FLinearColor::Black, DrawDebugDuration);

	if (!bResult)
	{
		return nullptr;
	}

	return Hit.GetComponent();
}

void UAbilityTask_WaitTraversableObstacle::CalculateLedges(const FCharacterMovementTraversalCheckInputs& Inputs, FCharacterMovementTraversalCheckResult& InOutResult)
{
}

bool UAbilityTask_WaitTraversableObstacle::CheckSpaceAboveObstacle(FCharacterMovementTraversalCheckResult& InOutResult)
{
	AActor* Context = GetAvatarActor();
	const TArray<AActor*> ActorsToIgnore = { Context };

	const FVector TraceStart = ActorLocation;
	const FVector ExtraSpace = FVector(0.f, 0.f, CapsuleHalfHeight + 2.f);
	FrontLedgeLocationCheck = InOutResult.FrontLedgeLocation + (InOutResult.FrontLedgeNormal * (CapsuleRadius + 2.f)) + ExtraSpace;

	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	const EDrawDebugTrace::Type DebugType = DrawDebugLevel >= 3 ? DebugTrace : EDrawDebugTrace::None;
	static constexpr bool bTraceComplex = false;
	static constexpr bool bIgnoreSelf = true;

	FHitResult Hit;
	UKismetSystemLibrary::CapsuleTraceSingle(Context, TraceStart, FrontLedgeLocationCheck, CapsuleRadius, CapsuleHalfHeight, TraceChannel,
		bTraceComplex, ActorsToIgnore, DebugType, Hit, bIgnoreSelf,
		FLinearColor::Red, FLinearColor::Green, DrawDebugDuration);

	// A "NOR" check between these two flags will indicate that we have room. 
	const bool bHasRoom = !(Hit.bBlockingHit || Hit.bStartPenetrating);
	if (bHasRoom)
	{
		const FVector TotalHeight = ActorLocation - FVector(0.f, 0.f, CapsuleHalfHeight) - InOutResult.FrontLedgeLocation; 
		InOutResult.ObstacleHeight = FMath::Abs(TotalHeight.Z);
	}
	
	return bHasRoom;
}

bool UAbilityTask_WaitTraversableObstacle::CheckSpaceAcrossObstacle(FCharacterMovementTraversalCheckResult& InOutResult)
{
	AActor* Context = GetAvatarActor();
	const TArray<AActor*> ActorsToIgnore = { Context };

	BackLedgeLocationCheck = InOutResult.BackLedgeLocation + (InOutResult.BackLedgeNormal * (CapsuleRadius + 2.f)) + InOutResult.FrontLedgeLocation;

	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	const EDrawDebugTrace::Type DebugType = DrawDebugLevel >= 3 ? DebugTrace : EDrawDebugTrace::None;
	static constexpr bool bTraceComplex = false;
	static constexpr bool bIgnoreSelf = true;

	FHitResult Hit;
	const bool bResult = UKismetSystemLibrary::CapsuleTraceSingle(Context, FrontLedgeLocationCheck, BackLedgeLocationCheck, CapsuleRadius, CapsuleHalfHeight, TraceChannel,
		bTraceComplex, ActorsToIgnore, DebugType, Hit, bIgnoreSelf,
		FLinearColor::Red, FLinearColor::Green, DrawDebugDuration);

	if (bResult)
	{
		// We found the back ledge within the sweep, so there is no room to go across. Depth is difference between front ledge and impact.
		InOutResult.bHasBackLedge = false;
		InOutResult.ObstacleDepth = (Hit.ImpactPoint - InOutResult.FrontLedgeLocation).Size2D();
	}
	else
	{
		// There is room past it so we'll probably vault it. Depth is the difference between front and back ledges.
		InOutResult.bHasBackLedge = true;
		InOutResult.ObstacleDepth = (InOutResult.FrontLedgeLocation - InOutResult.BackLedgeLocation).Size2D();
	}

	// We need to invert the result since **not** having a hit means we **do** have space.
	return !bResult;
}

bool UAbilityTask_WaitTraversableObstacle::CheckFloorBehindObstacle(FCharacterMovementTraversalCheckResult& InOutResult)
{
	AActor* Context = GetAvatarActor();
	const TArray<AActor*> ActorsToIgnore = { Context };

	const FVector TraceEnd = InOutResult.BackLedgeLocation + (InOutResult.BackLedgeNormal * (CapsuleRadius + 2.f)) - FVector(0.f, 0.f, 50.f);

	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	const EDrawDebugTrace::Type DebugType = DrawDebugLevel >= 3 ? DebugTrace : EDrawDebugTrace::None;
	static constexpr bool bTraceComplex = false;
	static constexpr bool bIgnoreSelf = true;

	FHitResult Hit;
	UKismetSystemLibrary::CapsuleTraceSingle(Context, BackLedgeLocationCheck, TraceEnd, CapsuleRadius, CapsuleHalfHeight, TraceChannel,
		bTraceComplex, ActorsToIgnore, DebugType, Hit, bIgnoreSelf,
		FLinearColor::Red, FLinearColor::Green, DrawDebugDuration);

	if (!Hit.bBlockingHit)
	{
		InOutResult.bHasBackFloor = false;
		return false;
	}

	InOutResult.bHasBackFloor = true;
	InOutResult.BackFloorLocation = Hit.ImpactPoint;
	InOutResult.BackLedgeHeight = FMath::Abs((Hit.ImpactPoint - InOutResult.BackLedgeLocation).Z);
	return true;
}

bool UAbilityTask_WaitTraversableObstacle::ChooseAnimationForTraversalAction(FCharacterMovementTraversalCheckResult& InOutResult)
{
	const AActor* Avatar = GetAvatarActor();
	
	FCharacterMovementTraversalChooserInput Input;
	Input.Action = InOutResult.Action;
	Input.bHasFrontLedge = InOutResult.bHasFrontLedge;
	Input.bHasBackLedge = InOutResult.bHasBackLedge;
	Input.bHasBackFloor = InOutResult.bHasBackFloor;
	Input.ObstacleHeight = InOutResult.ObstacleHeight;
	Input.ObstacleDepth = InOutResult.ObstacleDepth;
	Input.BackLedgeHeight = InOutResult.BackLedgeHeight;
	Input.MovementMode = CharacterAvatar->GetCharacterMovement()->MovementMode;
	Input.Gait = IAdvancedCharacterMovementInterface::Execute_GetCharacterGait(Avatar);
	Input.Speed = CharacterAvatar->GetCharacterMovement()->Velocity.Size2D();
	
	FCharacterMovementTraversalChooserOutput Output;
	
	FChooserEvaluationContext ChooserEvaluationContext = UChooserFunctionLibrary::MakeChooserEvaluationContext();
	ChooserEvaluationContext.AddStructParam(Input);
	ChooserEvaluationContext.AddStructParam(Output);
	
	const FInstancedStruct ChooserInstance = UChooserFunctionLibrary::MakeEvaluateChooser(ChooserTable);
	const TArray<UObject*> ChooserObjects = UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(ChooserEvaluationContext, ChooserInstance, UAnimationAsset::StaticClass());

	InOutResult.Action = Output.Action;
	if (InOutResult.Action == ECharacterTraversalAction::None)
	{
		return false;	
	}

	if (!IsValid(AnimInstance) && Avatar->Implements<UCombatSystemInterface>())
	{
		AnimInstance = ICombatSystemInterface::Execute_GetCombatAnimInstance(Avatar);
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
