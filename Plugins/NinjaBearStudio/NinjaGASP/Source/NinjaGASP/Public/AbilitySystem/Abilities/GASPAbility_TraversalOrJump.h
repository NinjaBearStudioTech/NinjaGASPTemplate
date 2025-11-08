// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaGASPBaseLocomotionAbility.h"
#include "GASPAbility_TraversalOrJump.generated.h"

class UChooserTable;

/**
 * Handles the traversal check and action. If that fails, leads to a jump.
 */
UCLASS()
class NINJAGASP_API UGASPAbility_TraversalOrJump : public UNinjaGASPBaseLocomotionAbility
{
	
	GENERATED_BODY()

public:

	UGASPAbility_TraversalOrJump();

protected:

	/** Trace type for the traversal object. It's recommended to create a dedicated channel for this! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
	TEnumAsByte<ETraceTypeQuery> TraceType;
	
	/** Chooser table including all traversal animations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UChooserTable> TraversalAnimations;
	
};
