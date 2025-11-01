// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "NinjaGASPPlayerController.generated.h"

class ANinjaGASPCharacter;
class UNinjaInputManagerComponent;

/**
 * Player controller compatible with Input and Factions frameworks.
 */
UCLASS()
class NINJAGASP_API ANinjaGASPPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	
	GENERATED_BODY()

public:

	ANinjaGASPPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Player Controller implementation
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnRep_Pawn() override;
	// -- End Player Controller implementation
	
	// -- Begin Generic Team Agent implementation
	virtual FGenericTeamId GetGenericTeamId() const override;
	// -- End Generic Team Agent implementation
	
	/**
	 * Changes the character possessed by this controller.
	 */
	UFUNCTION(BlueprintCallable, Category = "GASP|Player Controller")
	virtual void SpawnCharacter(const TSubclassOf<APawn>& NewPawnClass);
	
protected:

	static FName InputManagerName;

	/**
	 * Default player Team ID.
	 *
	 * The value set here can be used as a default, but it can be overwritten, in this order:
	 *
	 * 1. If the possessed pawn is a Faction Member and provides an ID using the Faction Component. 
	 * 2. If the possessed pawn implements IGenericTeamAgentInterface and provides a value.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extended Player Controller")
	uint8 PlayerTeamId;
	
	/** Blocks or allow dynamic character swapping, as implemented by the GASP sample. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extended Player Controller")
	bool bAllowDynamicCharacterSwap;
	
	/**
	 * Retrieves the Team ID from the currently possessed pawn.
	 * The value is retrieved using IGenericTeamAgentInterface.
	 */
	void CacheTeamIdFromPlayer();
	
	/**
	 * Changes and possesses the character in the server.
	 * If dynamic swapping is not allowed, the requesting player will be disconnected.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnCharacter(TSubclassOf<APawn> NewPawnClass);

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
