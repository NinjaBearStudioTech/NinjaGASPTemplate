// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPPlayerController.h"

#include "Components/NinjaInputManagerComponent.h"

FName ANinjaGASPPlayerController::InputManagerName = TEXT("InputManager");

ANinjaGASPPlayerController::ANinjaGASPPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InputManager = CreateDefaultSubobject<UNinjaInputManagerComponent>(InputManagerName);
}
