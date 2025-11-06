// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FAnimationStateMachineControlFlags.generated.h"

/**
 * Aggregates values that are used in transitions.
 */
USTRUCT(BlueprintType)
struct FAnimationStateMachineControlFlags
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation FSM Control Flags", DisplayName = "No Valid Animation")
	bool bNoValidAnimation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation FSM Control Flags", DisplayName = "Notify Transition Re-Transition")
	bool bNotifyTransitionReTransition = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation FSM Control Flags", DisplayName = "Notify Transition To Loop")
	bool bNotifyTransitionToLoop = false;

	void Reset()
	{
		bNoValidAnimation = false;
		bNotifyTransitionReTransition = false;
		bNotifyTransitionToLoop = false;
	}
};
