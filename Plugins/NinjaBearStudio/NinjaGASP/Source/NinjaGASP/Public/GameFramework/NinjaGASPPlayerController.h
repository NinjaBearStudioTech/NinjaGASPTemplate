// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NinjaGASPPlayerController.generated.h"

class UNinjaInputManagerComponent;

/**
 * Player controller compatible with Input and Factions frameworks.
 */
UCLASS()
class NINJAGASP_API ANinjaGASPPlayerController : public APlayerController
{
	
	GENERATED_BODY()

public:

	ANinjaGASPPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	static FName InputManagerName;
	
private:
	
	/**
	 * Routes inputs from actions/triggers to their handlers.
	 *
	 * Can receive shared input setups, or collect input setups from the possessed
	 * pawn, which is a good way to support changes such as player, mounts and so on.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Components, meta = (AllowPrivateAccess))
	TObjectPtr<UNinjaInputManagerComponent> InputManager;
	
};
