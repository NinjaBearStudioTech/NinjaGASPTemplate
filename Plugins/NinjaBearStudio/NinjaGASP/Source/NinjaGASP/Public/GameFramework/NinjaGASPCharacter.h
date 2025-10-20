// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NinjaGASCharacter.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"
#include "Interfaces/PreMovementComponentTickInterface.h"
#include "Interfaces/TraversalMovementInputInterface.h"
#include "NinjaGASPCharacter.generated.h"

/**
 * Base character, with functionality that can be shared between players and AI.
 *
 * Provides the boilerplate code necessary for the Gameplay Ability System, but Player Classes
 * using a Player State might need to perform certain extensions to disable that (this framework
 * provides an appropriate class to that end)
 */
UCLASS(Abstract)
class NINJAGASP_API ANinjaGASPCharacter : public ANinjaGASCharacter, public IPreMovementComponentTickInterface,
	public IAdvancedCharacterMovementInterface, public ITraversalMovementInputInterface
{
	
	GENERATED_BODY()

public:

	ANinjaGASPCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

};
