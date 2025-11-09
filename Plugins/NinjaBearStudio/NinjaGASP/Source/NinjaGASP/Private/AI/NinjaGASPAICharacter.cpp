// Ninja Bear Studio Inc., all rights reserved.
#include "AI/NinjaGASPAICharacter.h"

#include "AI/NinjaGASPAIController.h"

ANinjaGASPAICharacter::ANinjaGASPAICharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ANinjaGASPAIController::StaticClass();
}
