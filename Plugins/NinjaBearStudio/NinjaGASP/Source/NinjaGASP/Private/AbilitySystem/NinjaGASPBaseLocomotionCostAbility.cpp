// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/NinjaGASPBaseLocomotionCostAbility.h"

#include "TimerManager.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "AbilitySystem/Tasks/AbilityTask_NetworkSyncPointWithTimeout.h"
#include "Engine/World.h"

UNinjaGASPBaseLocomotionCostAbility::UNinjaGASPBaseLocomotionCostAbility()
{
	bChangedLocomotionMode = false;
	CostApplicationInterval = 1.f;
	MaxServerWaitTime = 1.5f;
}

bool UNinjaGASPBaseLocomotionCostAbility::CanActivateLocomotionMode_Implementation() const
{
	return true;
}

bool UNinjaGASPBaseLocomotionCostAbility::CanKeepLocomotionModeActive_Implementation() const
{
	return CanActivateLocomotionMode();
}

bool UNinjaGASPBaseLocomotionCostAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)
		&& CanActivateLocomotionMode();
}

void UNinjaGASPBaseLocomotionCostAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Initialize the recurring cost, only if we have a cost gameplay effect set.
	// Otherwise, we want this base functionality to be transparent to subclasses.
	//
	if (CostGameplayEffectClass != nullptr)
	{
		if (CommitAbilityCost(Handle, ActorInfo, ActivationInfo))
		{
			bChangedLocomotionMode = ActivateLocomotionMode();
			if (bChangedLocomotionMode)
			{
				ScheduleNetSync();
			}
		}
	}
	else
	{
		// Activate right away without checking for the cost (not set).
		bChangedLocomotionMode = ActivateLocomotionMode();
	}

	if (!bChangedLocomotionMode)
	{
		static constexpr bool bReplicateAbilityEnd = true;
		static constexpr bool bWasCancelled = false;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateAbilityEnd, bWasCancelled);
	}
}

void UNinjaGASPBaseLocomotionCostAbility::ScheduleNetSync()
{
	const UWorld* World = GetWorld();
	if (IsValid(World))
	{
		const FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::InitializeNetSync); 
		World->GetTimerManager().SetTimer(NetSyncScheduledTimer, Delegate, CostApplicationInterval, false);
	}
}

void UNinjaGASPBaseLocomotionCostAbility::InitializeNetSync()
{
	if (IsValid(NetSync))
	{
		NetSync->OnSync.RemoveAll(this);
		FinishLatentTask(NetSync);
	}
	
	NetSync = UAbilityTask_NetworkSyncPointWithTimeout::WaitNetSyncWithTimeout(this, EAbilityTaskNetSyncType::OnlyServerWait, MaxServerWaitTime);
	NetSync->OnSync.AddUniqueDynamic(this, &ThisClass::OnServerSynchronized);
	NetSync->ReadyForActivation();
}

void UNinjaGASPBaseLocomotionCostAbility::OnServerSynchronized()
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	
	if (CommitAbilityCost(Handle, ActorInfo, ActivationInfo) && CanKeepLocomotionModeActive())
	{
		ScheduleNetSync();
		return;
	}

	static constexpr bool bReplicateAbilityEnd = true;
	static constexpr bool bWasCancelled = false;
	EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateAbilityEnd, bWasCancelled);
}

void UNinjaGASPBaseLocomotionCostAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	const UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GetTimerManager().ClearTimer(NetSyncScheduledTimer);
		NetSyncScheduledTimer.Invalidate();
	}
	
	if (bChangedLocomotionMode)
	{
		static constexpr bool bForceCooldown = false;
		CommitAbilityCooldown(Handle, ActorInfo, ActivationInfo, bForceCooldown);
		DeactivateLocomotionMode();	
	}
	
	FinishLatentTask(NetSync);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UNinjaGASPBaseLocomotionCostAbility::ActivateLocomotionMode_Implementation()
{
	return false;
}

void UNinjaGASPBaseLocomotionCostAbility::DeactivateLocomotionMode_Implementation()
{
}