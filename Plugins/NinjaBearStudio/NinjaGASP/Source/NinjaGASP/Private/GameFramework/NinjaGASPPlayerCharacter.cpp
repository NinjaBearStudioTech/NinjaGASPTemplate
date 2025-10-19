// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPPlayerCharacter.h"

#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/PlayerPossessedAwareComponentInterface.h"

ANinjaGASPPlayerCharacter::ANinjaGASPPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		.DoNotCreateDefaultSubobject(AbilitySystemComponentName))
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
	const APlayerState* CurrentPlayerState = GetPlayerState();
	if (IsValid(CurrentPlayerState))
	{
		CharacterAbilitiesPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CurrentPlayerState);
	}
	else
	{
		CharacterAbilitiesPtr.Reset();
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
