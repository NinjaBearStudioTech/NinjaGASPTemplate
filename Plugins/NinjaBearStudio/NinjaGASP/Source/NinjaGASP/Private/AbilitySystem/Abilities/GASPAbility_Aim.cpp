// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/GASPAbility_Aim.h"

#include "NinjaGASPTags.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"

UGASPAbility_Aim::UGASPAbility_Aim()
{
	FGameplayTagContainer Tags;
	Tags.AddTagFast(Tag_GASP_Ability_Aim);
	SetAssetTags(Tags);
}

bool UGASPAbility_Aim::ActivateLocomotionMode_Implementation()
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (!IsValid(MyAvatar) || !MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		return false;
	}
	
	static constexpr bool bWantsToAim = true;
	IAdvancedCharacterMovementInterface::Execute_SetAimingIntent(MyAvatar, bWantsToAim);
	return true;
}

void UGASPAbility_Aim::DeactivateLocomotionMode_Implementation()
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (IsValid(MyAvatar) && MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		static constexpr bool bWantsToAim = false;
		IAdvancedCharacterMovementInterface::Execute_SetAimingIntent(MyAvatar, bWantsToAim);
	}
}
