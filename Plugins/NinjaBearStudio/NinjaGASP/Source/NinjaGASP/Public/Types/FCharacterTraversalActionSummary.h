// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECharacterTraversalAction.h"
#include "FCharacterTraversalActionSummary.generated.h"

/**
 * Summary of an active traversal action.
 */
USTRUCT(BlueprintType)
struct NINJAGASP_API FCharacterTraversalActionSummary
{
	
	GENERATED_BODY()

	/** The action being executed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Action Summary")
	ECharacterTraversalAction Action = ECharacterTraversalAction::None;

	/** Primitive component representing the target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal Action Summary")
	TWeakObjectPtr<UPrimitiveComponent> Target = nullptr;
	
};