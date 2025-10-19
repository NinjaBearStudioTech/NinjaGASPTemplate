// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NinjaGASPCharacterMovementComponent.generated.h"

/**
 * Character movement component with GASP-related extension points and custom movement modes. 
 */
UCLASS(ClassGroup=(NinjaGASP), meta=(BlueprintSpawnableComponent))
class NINJAGASP_API UNinjaGASPCharacterMovementComponent : public UCharacterMovementComponent
{
	
	GENERATED_BODY()

public:

	// Begin CMC implementation
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End CMC implementation

protected:

	/**
	 * Allows the character to execute any pre-tick logic, before this component's Tick.
	 * The owner must implement PreMovementComponentTickInterface. 
	 */
	void ExecutePreTickExtensionOnOwningCharacter() const;
	
};
