// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPPlayerCharacter.h"

#include "AbilitySystemGlobals.h"
#include "NinjaInventoryFunctionLibrary.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "Components/NinjaInventoryManagerComponent.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/PlayerPossessedAwareComponentInterface.h"

ANinjaGASPPlayerCharacter::ANinjaGASPPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		.DoNotCreateDefaultSubobject(AbilitySystemComponentName)
		.DoNotCreateDefaultSubobject(InventoryManagerName))
{
	bInitializeAbilityComponentOnBeginPlay = false;
	NetPriority = 3.f;
	AutoPossessAI = EAutoPossessAI::Disabled;
	AbilityReplicationMode = EGameplayEffectReplicationMode::Mixed;

	SetMinNetUpdateFrequency(33.f);
	SetNetUpdateFrequency(66.f);	
}

void ANinjaGASPPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeFromPlayerState();
	
	TArray<UActorComponent*> PlayerPossessedAwareComponents = GetComponentsByInterface(UPlayerPossessedAwareComponentInterface::StaticClass());
	for (UActorComponent* Component: PlayerPossessedAwareComponents)
	{
		IPlayerPossessedAwareComponentInterface* Interface = Cast<IPlayerPossessedAwareComponentInterface>(Component);
		Interface->OnPossessed(NewController, this);
	}
}

void ANinjaGASPPlayerCharacter::InitializeFromPlayerState()
{
	APlayerState* CurrentPlayerState = GetPlayerState();
	InitializeAbilitySystemComponentFromPlayerState(CurrentPlayerState);
	InitializeInventorySystemComponentFromPlayerState(CurrentPlayerState);
}

void ANinjaGASPPlayerCharacter::InitializeAbilitySystemComponentFromPlayerState(APlayerState* CurrentPlayerState)
{
	if (IsValid(CurrentPlayerState))
	{
		CharacterAbilitiesPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CurrentPlayerState);
		SetupAbilitySystemComponent(CurrentPlayerState);
	}
	else
	{
		ClearAbilitySystemComponent();
		CharacterAbilitiesPtr.Reset();
	}	
}

void ANinjaGASPPlayerCharacter::InitializeInventorySystemComponentFromPlayerState(APlayerState* CurrentPlayerState)
{
	if (IsValid(CurrentPlayerState))
	{
		CharacterInventoryPtr = UNinjaInventoryFunctionLibrary::GetInventoryManager(CurrentPlayerState);
	}
	else
	{
		CharacterInventoryPtr.Reset();
	}	
}

void ANinjaGASPPlayerCharacter::SetupAbilitySystemComponent(AActor* AbilitySystemOwner)
{
	if (IsValid(AbilitySystemOwner))
	{
		UAbilitySystemComponent* NewAbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilitySystemOwner);
		if (IsValid(NewAbilityComponent))
		{
			NewAbilityComponent->InitAbilityActorInfo(AbilitySystemOwner, this);
			CharacterAbilitiesPtr = NewAbilityComponent;
		}
	}
}

UAbilitySystemComponent* ANinjaGASPPlayerCharacter::GetAbilitySystemComponent() const
{
	if (CharacterAbilitiesPtr.IsValid() && CharacterAbilitiesPtr->IsValidLowLevelFast())
	{
		return CharacterAbilitiesPtr.Get();
	}
	return nullptr;
}

UNinjaInventoryManagerComponent* ANinjaGASPPlayerCharacter::GetInventoryManager_Implementation() const
{
	if (CharacterInventoryPtr.IsValid() && CharacterInventoryPtr->IsValidLowLevelFast())
	{
		return CharacterInventoryPtr.Get();
	}
	return nullptr;
}

void ANinjaGASPPlayerCharacter::OnInventoryInitializationFinished_Implementation(UNinjaInventoryManagerComponent* Inventory)
{
	CharacterInventoryPtr = Inventory;
	Super::OnInventoryInitializationFinished_Implementation(Inventory);
}

void ANinjaGASPPlayerCharacter::SetCameraAiming_Implementation(const bool bNewCameraAiming)
{
	// Just relay the request to the intent.
	Execute_SetAimingIntent(this, bNewCameraAiming);
}

void ANinjaGASPPlayerCharacter::UnPossessed()
{
	ClearAbilitySystemComponent();
	Super::UnPossessed();
}

void ANinjaGASPPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeFromPlayerState();
}
