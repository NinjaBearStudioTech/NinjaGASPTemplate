// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPPlayerState.h"

#include "Components/NinjaInventoryManagerComponent.h"

FName ANinjaGASPPlayerState::InventoryManagerName = TEXT("InventoryManager");

ANinjaGASPPlayerState::ANinjaGASPPlayerState()
{
	InventoryManager = CreateDefaultSubobject<UNinjaInventoryManagerComponent>(InventoryManagerName);
}
