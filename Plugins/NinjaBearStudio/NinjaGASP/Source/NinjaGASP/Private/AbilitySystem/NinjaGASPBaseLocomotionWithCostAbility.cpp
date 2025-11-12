// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/NinjaGASPBaseLocomotionWithCostAbility.h"

#include "TimerManager.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "AbilitySystem/Tasks/AbilityTask_NetworkSyncPointWithTimeout.h"
#include "Engine/World.h"

UNinjaGASPBaseLocomotionWithCostAbility::UNinjaGASPBaseLocomotionWithCostAbility()
{
	CostApplicationInterval = 1.f;
	MaxServerWaitTime = 1.5f;
}

bool UNinjaGASPBaseLocomotionWithCostAbility::CanKeepLocomotionModeActive_Implementation() const
{
	return CanActivateLocomotionMode();
}

void UNinjaGASPBaseLocomotionWithCostAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Skip the parent, so we can readjust how cost is applied.
	UNinjaGASGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (CostGameplayEffectClass != nullptr)
	{
		const bool bHasBudget = ShouldApplyCost() ? CommitAbilityCost(Handle, ActorInfo, ActivationInfo) : true;
		if (bHasBudget)
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

void UNinjaGASPBaseLocomotionWithCostAbility::ScheduleNetSync()
{
	const UWorld* World = GetWorld();
	if (IsValid(World))
	{
		const FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ThisClass::InitializeNetSync); 
		World->GetTimerManager().SetTimer(NetSyncScheduledTimer, Delegate, CostApplicationInterval, false);
	}
}

void UNinjaGASPBaseLocomotionWithCostAbility::InitializeNetSync()
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

void UNinjaGASPBaseLocomotionWithCostAbility::OnServerSynchronized()
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();

	const bool bHasBudget = ShouldApplyCost() ? CommitAbilityCost(Handle, ActorInfo, ActivationInfo) : true;
	if (bHasBudget && CanKeepLocomotionModeActive())
	{
		ScheduleNetSync();
		return;
	}

	static constexpr bool bReplicateAbilityEnd = true;
	static constexpr bool bWasCancelled = false;
	EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateAbilityEnd, bWasCancelled);
}

bool UNinjaGASPBaseLocomotionWithCostAbility::ShouldApplyCost_Implementation() const
{
	return true;
}

void UNinjaGASPBaseLocomotionWithCostAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	const UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GetTimerManager().ClearTimer(NetSyncScheduledTimer);
		NetSyncScheduledTimer.Invalidate();
	}
	
	FinishLatentTask(NetSync);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
