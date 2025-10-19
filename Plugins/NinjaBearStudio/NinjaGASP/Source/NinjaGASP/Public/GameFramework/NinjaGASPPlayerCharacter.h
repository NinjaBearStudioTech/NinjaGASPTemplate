// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaGASPCharacter.h"
#include "Interfaces/TraversalMovementInputInterface.h"
#include "NinjaGASPPlayerCharacter.generated.h"

class UInputAction;
class UAbilitySystemComponent;

/**
 * Player character that adds camera and input features.
 *
 * It also modifies the base class to support an Ability System Component assigned
 * to the Player State, covering the expected deferred initialization.
 */
UCLASS(Abstract)
class NINJAGASP_API ANinjaGASPPlayerCharacter : public ANinjaGASPCharacter, public ITraversalMovementInputInterface
{
	
	GENERATED_BODY()

public:

	ANinjaGASPPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Character implementation
	virtual void PossessedBy(AController* NewController) override;
	virtual void InitializeFromPlayerState() override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupAbilitySystemComponent(AActor* AbilitySystemOwner) override;
	// -- End Character implementation

private:

	/** Weak reference to the Character's Ability Component. */
	TWeakObjectPtr<UAbilitySystemComponent> CharacterAbilitiesPtr;	
	
};
