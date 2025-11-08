// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/GASPAbility_Walk.h"

#include "NinjaGASPTags.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"

UGASPAbility_Walk::UGASPAbility_Walk()
{
	FGameplayTagContainer Tags;
	Tags.AddTagFast(Tag_GASP_Ability_Walk);
	SetAssetTags(Tags);

	CancelAbilitiesWithTag.AddTagFast(Tag_GASP_Ability_Jog);
	CancelAbilitiesWithTag.AddTagFast(Tag_GASP_Ability_Sprint);
}

bool UGASPAbility_Walk::IsWalking() const
{
	const AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (IsValid(MyAvatar) && MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		return IAdvancedCharacterMovementInterface::Execute_IsWalking(MyAvatar);
	}

	return false;
}

bool UGASPAbility_Walk::ActivateLocomotionMode_Implementation()
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (!IsValid(MyAvatar) || !MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		return false;
	}
	
	static constexpr bool bWantsToWalk = true;
	IAdvancedCharacterMovementInterface::Execute_SetWalkingIntent(MyAvatar, bWantsToWalk);
	return true;
}

void UGASPAbility_Walk::DeactivateLocomotionMode_Implementation()
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (IsValid(MyAvatar) && MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		static constexpr bool bWantsToWalk = false;
		IAdvancedCharacterMovementInterface::Execute_SetWalkingIntent(MyAvatar, bWantsToWalk);
	}
}
