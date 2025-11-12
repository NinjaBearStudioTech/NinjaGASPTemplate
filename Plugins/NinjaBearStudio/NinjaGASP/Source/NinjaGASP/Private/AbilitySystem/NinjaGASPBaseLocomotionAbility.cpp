// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/NinjaGASPBaseLocomotionAbility.h"

UNinjaGASPBaseLocomotionAbility::UNinjaGASPBaseLocomotionAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bChangedLocomotionMode = false;
}

bool UNinjaGASPBaseLocomotionAbility::CanActivateLocomotionMode_Implementation() const
{
	return true;
}

bool UNinjaGASPBaseLocomotionAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)
		&& CanActivateLocomotionMode();
}

void UNinjaGASPBaseLocomotionAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CostGameplayEffectClass != nullptr)
	{
		if (CommitAbilityCost(Handle, ActorInfo, ActivationInfo))
		{
			bChangedLocomotionMode = ActivateLocomotionMode();
		}
	}
	else
	{
		// Activate right away without checking for the cost (not set).
		bChangedLocomotionMode = ActivateLocomotionMode();
	}

	if (bChangedLocomotionMode)
	{
		ApplyLocomotionEffect();
	}
	else
	{
		static constexpr bool bReplicateAbilityEnd = true;
		static constexpr bool bWasCancelled = false;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateAbilityEnd, bWasCancelled);
	}
}

bool UNinjaGASPBaseLocomotionAbility::ActivateLocomotionMode_Implementation()
{
	return false;
}

void UNinjaGASPBaseLocomotionAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	if (bChangedLocomotionMode)
	{
		static constexpr bool bForceCooldown = false;
		CommitAbilityCooldown(Handle, ActorInfo, ActivationInfo, bForceCooldown);
		DeactivateLocomotionMode();	
	}

	RemoveLocomotionEffect();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UNinjaGASPBaseLocomotionAbility::DeactivateLocomotionMode_Implementation()
{
}

void UNinjaGASPBaseLocomotionAbility::ApplyLocomotionEffect()
{
	if (LocomotionEffectClass)
	{
		const int32 Level = GetAbilityLevel();
		const FGameplayEffectSpecHandle TargetLockSpecHandle = MakeOutgoingGameplayEffectSpec(LocomotionEffectClass, Level);
		LocomotionGameplayEffectHandle = K2_ApplyGameplayEffectSpecToOwner(TargetLockSpecHandle);	
	}
}

void UNinjaGASPBaseLocomotionAbility::RemoveLocomotionEffect()
{
	if (LocomotionGameplayEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(LocomotionGameplayEffectHandle);
	}
}
