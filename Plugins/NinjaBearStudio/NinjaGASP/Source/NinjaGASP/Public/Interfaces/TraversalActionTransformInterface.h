// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TraversalActionTransformInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTraversalActionTransformInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Gets and sets a transform calculated for a traversal action.
 */
class NINJAGASP_API ITraversalActionTransformInterface
{
	
	GENERATED_BODY()

public:

	/**
	 * Provides the traversal transform set. It should always be thread safe.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Traversal Transform Interface", meta = (BlueprintThreadSafe))
	FTransform GetTraversalTransform() const;

	/**
	 * Sets a new traversal transform. 
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Traversal Transform Interface")
	void SetTraversalTransform(FTransform NewTransform);
	
};
