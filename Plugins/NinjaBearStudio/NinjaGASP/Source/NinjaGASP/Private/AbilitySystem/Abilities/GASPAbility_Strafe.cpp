// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/GASPAbility_Strafe.h"

#include "NinjaGASPTags.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"

UGASPAbility_Strafe::UGASPAbility_Strafe()
{
	FGameplayTagContainer Tags;
	Tags.AddTagFast(Tag_GASP_Ability_Strafe);
	SetAssetTags(Tags);
}

bool UGASPAbility_Strafe::ActivateLocomotionMode_Implementation()
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (!IsValid(MyAvatar) || !MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		return false;
	}
	
	static constexpr bool bWantsToStrafe = true;
	IAdvancedCharacterMovementInterface::Execute_SetStrafingIntent(MyAvatar, bWantsToStrafe);
	return true;
}

void UGASPAbility_Strafe::DeactivateLocomotionMode_Implementation()
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (IsValid(MyAvatar) && MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		static constexpr bool bWantsToStrafe = false;
		IAdvancedCharacterMovementInterface::Execute_SetStrafingIntent(MyAvatar, bWantsToStrafe);
	}
}
