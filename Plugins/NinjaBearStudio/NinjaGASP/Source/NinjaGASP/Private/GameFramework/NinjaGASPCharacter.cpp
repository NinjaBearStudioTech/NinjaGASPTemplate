// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPCharacter.h"

#include "GameFramework/NinjaGASPCharacterMovementComponent.h"

ANinjaGASPCharacter::ANinjaGASPCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNinjaGASPCharacterMovementComponent>(CharacterMovementComponentName))
{
}
