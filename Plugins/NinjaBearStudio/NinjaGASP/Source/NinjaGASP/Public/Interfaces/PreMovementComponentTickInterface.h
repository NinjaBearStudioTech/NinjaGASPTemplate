// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PreMovementComponentTickInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPreMovementComponentTickInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Allows an owning character to trigger code that is guaranteed to run before its CMC.
 *
 * This is a substitution for the "Pre CMC Tick Component" introduced by GASP. That component
 * is a viable Blueprint alternative, but we can achieve the same results, using this C++ interface,
 * without the extra memory cost of a new Actor Component.
 */
class NINJAGASP_API IPreMovementComponentTickInterface
{
	
	GENERATED_BODY()

public:

	/**
	 * Extension point that can run any logic before the Character Movement Component tick.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Pre Movement Component Tick Interface")
	void PreMovementComponentTick();

};

