﻿// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/GASPAbility_Sprint.h"

#include "AbilitySystemComponent.h"
#include "NinjaGASPTags.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"

UGASPAbility_Sprint::UGASPAbility_Sprint()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FGameplayTagContainer Tags;
	Tags.AddTagFast(Tag_GASP_Ability_Sprint);
	SetAssetTags(Tags);
}

bool UGASPAbility_Sprint::IsSprinting() const
{
	const AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (!IsValid(MyAvatar) || MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		return false;
	}
	
	return IAdvancedCharacterMovementInterface::Execute_IsSprinting(MyAvatar);
}

bool UGASPAbility_Sprint::ActivateLocomotionMode_Implementation()
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (!IsValid(MyAvatar) || !MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		return false;
	}
	
	static constexpr bool bWantsToSprint = true;
	IAdvancedCharacterMovementInterface::Execute_SetSprintingIntent(MyAvatar, bWantsToSprint);
	return true;	
}

void UGASPAbility_Sprint::DeactivateLocomotionMode_Implementation()
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (IsValid(MyAvatar) && MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		static constexpr bool bWantsToSprint = false;
		IAdvancedCharacterMovementInterface::Execute_SetSprintingIntent(MyAvatar, bWantsToSprint);
	}
}

