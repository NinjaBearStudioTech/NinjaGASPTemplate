// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/GASPAbility_Traverse.h"

#include "NinjaGASPCollisions.h"
#include "NinjaGASPTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tasks/AbilityTask_WaitTraversableObstacle.h"
#include "GameFramework/NinjaGASPCharacter.h"

UGASPAbility_Traverse::UGASPAbility_Traverse()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	bStartedTraversalAction = false;
	TraversalChannel = COLLISION_TRAVERSAL;
	FallbackAbilityTags.AddTagFast(Tag_GASP_Ability_Jump); 
	TraversableTaskClass = UAbilityTask_WaitTraversableObstacle::StaticClass();

	FGameplayTagContainer Tags;
	Tags.AddTagFast(Tag_GASP_Ability_Traverse);
	SetAssetTags(Tags);
}

bool UGASPAbility_Traverse::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!TraversalAnimations || !TraversableTaskClass)
	{
		return false;
	}
		
	const ANinjaGASPCharacter* Character = Cast<ANinjaGASPCharacter>(GetAvatarActorFromActorInfo());
	if (IsValid(Character))
	{
		const FCharacterTraversalActionSummary TraversalSummary = Character->GetActiveTraversalAction();
		return TraversalSummary.Action == ECharacterTraversalAction::None;
	}

	return true;
}

void UGASPAbility_Traverse::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		TraversableTask = UAbilityTask_WaitTraversableObstacle::CreateTask(this, TraversalChannel, TraversalAnimations, TraversableTaskClass);
		TraversableTask->SetAnimInstance(GetAnimInstance());
		TraversableTask->OnTraversableObstacleSearchFinished.AddUniqueDynamic(this, &ThisClass::HandleTraversableObstacleSearchFinished);
		TraversableTask->ReadyForActivation();
	}
	else
	{
		static constexpr bool bReplicateEndAbility = true;
		static constexpr bool bWasCancelled = false;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UGASPAbility_Traverse::HandleTraversableObstacleSearchFinished(const FCharacterMovementTraversalCheckResult& Result)
{
	UAnimMontage* MontageToPlay = Result.MontageToPlay;
	if (Result.Outcome != ECharacterTraversalOutcome::TraversalFound || !MontageToPlay)
	{
		FallbackFromInvalidTraversal();
		K2_EndAbility();
		return;
	}

	ANinjaGASPCharacter* Character = Cast<ANinjaGASPCharacter>(GetAvatarActorFromActorInfo());
	if (IsValid(Character))
	{
		const ECharacterTraversalAction ActionType = Result.Action;
		UPrimitiveComponent* Target = Result.HitComponent;
		Character->RegisterTraversalAction(ActionType, Target);
	}
	
	const float PlayRate = Result.PlayRate;
	const float StartTime = Result.StartTime;

	static constexpr bool bStopWhenAbilityEnds = true;
	static constexpr bool bAllowInterruptAfterBlendOut = false;
	static constexpr float AnimRootMotionTranslationScale = 1;
	static const FName Section = NAME_None;
	
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("TraversalMontage"),
		MontageToPlay, PlayRate, Section, bStopWhenAbilityEnds, AnimRootMotionTranslationScale, StartTime, bAllowInterruptAfterBlendOut);

	MontageTask->OnCompleted.AddUniqueDynamic(this, &ThisClass::HandleMontageEnded);
	MontageTask->OnBlendOut.AddUniqueDynamic(this, &ThisClass::HandleMontageEnded);
	MontageTask->OnCancelled.AddUniqueDynamic(this, &ThisClass::HandleMontageCancelled);
	MontageTask->OnInterrupted.AddUniqueDynamic(this, &ThisClass::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
	
	bStartedTraversalAction = true;
}

void UGASPAbility_Traverse::FallbackFromInvalidTraversal_Implementation()
{
	if (FallbackAbilityTags.IsValid())
	{
		UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
		if (IsValid(AbilitySystem))
		{
			static constexpr bool bAllowRemoteActivation = true;
			AbilitySystem->TryActivateAbilitiesByTag(FallbackAbilityTags, bAllowRemoteActivation);
		}
	}
}

void UGASPAbility_Traverse::HandleMontageEnded()
{
	K2_EndAbility();
}

void UGASPAbility_Traverse::HandleMontageCancelled()
{
	K2_CancelAbility();
}

void UGASPAbility_Traverse::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	if (bStartedTraversalAction)
	{
		ANinjaGASPCharacter* Character = Cast<ANinjaGASPCharacter>(GetAvatarActorFromActorInfo());
		if (IsValid(Character))
		{
			Character->ClearTraversalAction();
		}
		bStartedTraversalAction = false;
	}
	
	FinishLatentTasks({ TraversableTask, MontageTask });
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
