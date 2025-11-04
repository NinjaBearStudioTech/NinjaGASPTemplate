// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaGASPCharacter.h"
#include "Interfaces/PlayerCameraModeInterface.h"
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
class NINJAGASP_API ANinjaGASPPlayerCharacter : public ANinjaGASPCharacter, public IPlayerCameraModeInterface
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
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UNinjaInventoryManagerComponent* GetInventoryManager_Implementation() const override;
	// -- End Character implementation

protected:

	/** Initializes (or clears) the Ability System, from the provided Player State. */
	virtual void InitializeAbilitySystemComponentFromPlayerState(APlayerState* CurrentPlayerState);

	/** Initializes (or clears) the Inventory Manager, from the provided Player State. */
	virtual void InitializeInventorySystemComponentFromPlayerState(APlayerState* CurrentPlayerState);
	
private:

	/** Weak reference to the Character's Ability Component. */
	TWeakObjectPtr<UAbilitySystemComponent> CharacterAbilitiesPtr;	

	/** Weak reference to the Character's Inventory Component. */
	TWeakObjectPtr<UNinjaInventoryManagerComponent> CharacterInventoryPtr;
	
};
