// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/GASPAbility_Crouch.h"

#include "NinjaGASPTags.h"
#include "GameFramework/Character.h"

UGASPAbility_Crouch::UGASPAbility_Crouch()
{
	FGameplayTagContainer Tags;
	Tags.AddTagFast(Tag_GASP_Ability_Crouch);
	SetAssetTags(Tags);

	BlockAbilitiesWithTag.AddTagFast(Tag_GASP_Ability_Jog);
	BlockAbilitiesWithTag.AddTagFast(Tag_GASP_Ability_Sprint);
	BlockAbilitiesWithTag.AddTagFast(Tag_GASP_Ability_Walk);
}

bool UGASPAbility_Crouch::ActivateLocomotionMode_Implementation()
{
	ACharacter* MyAvatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!IsValid(MyAvatar))
	{
		return false;
	}

	MyAvatar->Crouch();
	return true;
}

void UGASPAbility_Crouch::DeactivateLocomotionMode_Implementation()
{
	ACharacter* MyAvatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (IsValid(MyAvatar))
	{
		MyAvatar->UnCrouch();
	}
}
