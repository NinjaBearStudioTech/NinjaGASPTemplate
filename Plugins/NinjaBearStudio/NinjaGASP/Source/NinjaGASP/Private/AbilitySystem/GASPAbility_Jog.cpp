// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/GASPAbility_Jog.h"

#include "AbilitySystemComponent.h"
#include "NinjaGASPTags.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"

UGASPAbility_Jog::UGASPAbility_Jog()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FGameplayTagContainer Tags;
	Tags.AddTagFast(Tag_GASP_Ability_Jog);
	SetAssetTags(Tags);
	
	CancelAbilitiesWithTag.AddTagFast(Tag_GASP_Ability_Sprint);
	CancelAbilitiesWithTag.AddTagFast(Tag_GASP_Ability_Walk);

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

void UGASPAbility_Jog::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (!IsJogging() && CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		static constexpr bool bWantsToSprint = false;
		static constexpr bool bWantsToWalk = false;
		IAdvancedCharacterMovementInterface::Execute_SetSprintingIntent(MyAvatar, bWantsToSprint);
		IAdvancedCharacterMovementInterface::Execute_SetWalkingIntent(MyAvatar, bWantsToWalk);
	}
	else
	{
		static constexpr bool bReplicateCancel = true;
		CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancel);
	}	
}

void UGASPAbility_Jog::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	TryActivateWalkAbility();
}

void UGASPAbility_Jog::TryActivateWalkAbility() const
{
	UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponentFromActorInfo();
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->TryActivateAbilitiesByTag(WalkAbilityActivationTags);	
	}	
}
