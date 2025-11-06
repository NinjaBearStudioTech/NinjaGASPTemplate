// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "EAnimationStateMachineState.generated.h"

UENUM(BlueprintType, DisplayName = "Animation State Machine State")
enum class EAnimationStateMachineState : uint8
{
	IdleLoop						UMETA(DisplayName = "Idle Loop"),
	TransitionToIdleLoop			UMETA(DisplayName = "Transition to Idle Loop"),
	LocomotionLoop					UMETA(DisplayName = "Locomotion Loop"),
	TransitionToLocomotionLoop		UMETA(DisplayName = "Transition to Locomotion Loop"),
	InAirLoop						UMETA(DisplayName = "In Air Loop"),
	TransitionToInAirLoop			UMETA(DisplayName = "Transition to In Air Loop"),
	IdleBreak						UMETA(DisplayName = "Idle Break")
};