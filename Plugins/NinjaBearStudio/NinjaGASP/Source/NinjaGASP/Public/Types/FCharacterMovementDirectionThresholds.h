// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FCharacterMovementDirectionThresholds.generated.h"

/**
 * Thresholds used to determine the current movement direction.
 */
USTRUCT(BlueprintType)
struct NINJAGASP_API FCharacterMovementDirectionThresholds
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Thresholds")
	float ForwardLeft = -60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Thresholds")
	float ForwardRight = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Thresholds")
	float BackwardLeft = -120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Thresholds")
	float BackwardRight = 120.f;

	FCharacterMovementDirectionThresholds() = default;

	FCharacterMovementDirectionThresholds(const float ForwardLeft, const float ForwardRight, const float BackwardLeft, const float BackwardRight)
		: ForwardLeft(ForwardLeft), ForwardRight(ForwardRight), BackwardLeft(BackwardLeft), BackwardRight(BackwardRight)
	{
	}
};
