// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPCharacterMovementComponent.h"

#include "Interfaces/PreMovementComponentTickInterface.h"

UNinjaGASPCharacterMovementComponent::UNinjaGASPCharacterMovementComponent()
{
	bOrientRotationToMovement = true;
	NavMovementProperties.bUseAccelerationForPaths = true;
}

void UNinjaGASPCharacterMovementComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	ExecutePreTickExtensionOnOwningCharacter();
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UNinjaGASPCharacterMovementComponent::ExecutePreTickExtensionOnOwningCharacter() const
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner) && Owner->Implements<UPreMovementComponentTickInterface>())
	{
		IPreMovementComponentTickInterface::Execute_PreMovementComponentTick(Owner);
	}
}
