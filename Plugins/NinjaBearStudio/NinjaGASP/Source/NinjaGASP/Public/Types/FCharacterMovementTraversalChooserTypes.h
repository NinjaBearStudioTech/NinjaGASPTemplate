// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterGait.h"
#include "ECharacterTraversalAction.h"
#include "FCharacterMovementTraversalChooserTypes.generated.h"

/**
 * Input data used by the Traversal Chooser.
 */
USTRUCT(BlueprintType)
struct NINJAGASP_API FCharacterMovementTraversalChooserInput
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	ECharacterTraversalAction Action = ECharacterTraversalAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	bool bHasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	bool bHasBackLedge = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	bool bHasBackFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	float ObstacleHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	float ObstacleDepth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	float BackLedgeHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	TEnumAsByte<EMovementMode> MovementMode = MOVE_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	ECharacterGait Gait = ECharacterGait::Run;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Input")
	float Speed = 0.f;
	
};

/**
 * Output data used by the Traversal Chooser.
 */
USTRUCT(BlueprintType)
struct NINJAGASP_API FCharacterMovementTraversalChooserOutput
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Chooser Output")
	ECharacterTraversalAction Action = ECharacterTraversalAction::None;
	
};