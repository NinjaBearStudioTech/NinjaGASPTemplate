// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerPossessedAwareComponentInterface.generated.h"

class AController;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UPlayerPossessedAwareComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface that can be added to player components that must be informed about the possessed event.
 * The base player class must address any components implementing this interface.
 */
class NINJAGASP_API IPlayerPossessedAwareComponentInterface
{
	
	GENERATED_BODY()

public:

	/**
	 * Receives the Possessed event from the owning player character.
	 */
	virtual void OnPossessed(AController* Controller, ACharacter* Character) = 0;

};

