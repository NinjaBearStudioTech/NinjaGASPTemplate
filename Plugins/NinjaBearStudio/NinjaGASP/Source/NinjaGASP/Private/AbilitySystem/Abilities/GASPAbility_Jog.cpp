// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/GASPAbility_Jog.h"

#include "AbilitySystemComponent.h"
#include "NinjaCombatTags.h"
#include "NinjaGASPTags.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"

UGASPAbility_Jog::UGASPAbility_Jog()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FGameplayTagContainer Tags;
	Tags.AddTagFast(Tag_GASP_Ability_Jog);
	SetAssetTags(Tags);

	ActivationOwnedTags.AddTagFast(Tag_Combat_Effect_Cancel_Regeneration_Stamina);
	
	CancelAbilitiesWithTag.AddTagFast(Tag_GASP_Ability_Sprint);
	CancelAbilitiesWithTag.AddTagFast(Tag_GASP_Ability_Walk);

	bActivateWalkOnEnd = true;
	WalkAbilityActivationTags.AddTagFast(Tag_GASP_Ability_Walk);
}

bool UGASPAbility_Jog::IsJogging() const
{
	const AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (!IsValid(MyAvatar) || MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		return false;
	}
	
	if (IAdvancedCharacterMovementInterface::Execute_IsWalking(MyAvatar))
	{
		// The character is walking, hence not jogging.
		return false;
	}

	if (IAdvancedCharacterMovementInterface::Execute_IsSprinting(MyAvatar))
	{
		// The character is sprinting, hence not jogging.
		return false;
	}

	return true;
}

bool UGASPAbility_Jog::ActivateLocomotionMode_Implementation()
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (!IsValid(MyAvatar) || !MyAvatar->Implements<UAdvancedCharacterMovementInterface>())
	{
		return false;
	}
	
	static constexpr bool bWantsToSprint = false;
	static constexpr bool bWantsToWalk = false;
	IAdvancedCharacterMovementInterface::Execute_SetSprintingIntent(MyAvatar, bWantsToSprint);
	IAdvancedCharacterMovementInterface::Execute_SetWalkingIntent(MyAvatar, bWantsToWalk);
	return true;	
}

void UGASPAbility_Jog::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// Do this **after** the ability has ended to avoid potential ability tag clashes!
	if (bActivateWalkOnEnd && WalkAbilityActivationTags.IsValid())
	{
		UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponentFromActorInfo();
		if (IsValid(AbilityComponent))
		{
			AbilityComponent->TryActivateAbilitiesByTag(WalkAbilityActivationTags);	
		}	
	}
}
