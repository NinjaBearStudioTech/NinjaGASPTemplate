// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterTraversalAction.h"
#include "ECharacterTraversalOutcome.h"
#include "FCharacterMovementTraversalCheckResult.generated.h"

/**
 * Contains the data obtained from the traversal check.
 */
USTRUCT(BlueprintType)
struct NINJAGASP_API FCharacterMovementTraversalCheckResult
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	ECharacterTraversalOutcome Outcome = ECharacterTraversalOutcome::NotInitialized;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	ECharacterTraversalAction Action = ECharacterTraversalAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	uint8 bHasFrontLedge:1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	FVector FrontLedgeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	FVector FrontLedgeNormal = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	uint8 bHasBackLedge:1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	FVector BackLedgeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	FVector BackLedgeNormal = FVector::ZeroVector;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	uint8 bHasBackFloor:1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	FVector BackFloorLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	float ObstacleHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	float ObstacleDepth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	float BackLedgeHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Check Result")
	float PlayRate = 0.f;
	
};
