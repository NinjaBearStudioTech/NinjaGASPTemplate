// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/GASPAbility_Walk.h"

#include "NinjaGASPTags.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"

UGASPAbility_Walk::UGASPAbility_Walk()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
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

void UGASPAbility_Walk::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (!IsWalking() && CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		
		static constexpr bool bWantsToWalk = true;
		IAdvancedCharacterMovementInterface::Execute_SetWalkingIntent(MyAvatar, bWantsToWalk);
	}
	else
	{
		static constexpr bool bReplicateCancel = true;
		CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancel);
	}
}

void UGASPAbility_Walk::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	AActor* MyAvatar = GetAvatarActorFromActorInfo();
	if (IsWalking())
	{
		static constexpr bool bWantsToWalk = false;
		IAdvancedCharacterMovementInterface::Execute_SetWalkingIntent(MyAvatar, bWantsToWalk);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
