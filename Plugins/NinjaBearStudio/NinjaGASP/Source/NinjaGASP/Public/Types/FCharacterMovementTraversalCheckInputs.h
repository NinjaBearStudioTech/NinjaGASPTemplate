// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FCharacterMovementTraversalCheckInputs.generated.h"

/**
 * Evaluates inputs for a traversal movement.
 */
USTRUCT(BlueprintType)
struct NINJAGASP_API FCharacterMovementTraversalCheckInputs
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Inputs")
	FVector TraceForwardDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Inputs")
	float TraceForwardDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Inputs")
	FVector TraceOriginOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Inputs")
	FVector TraceEndOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Inputs")
	float TraceRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Inputs")
	float TraceHalfHeight = 0.f;

};