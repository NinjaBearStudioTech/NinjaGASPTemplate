// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPCharacter.h"

#include "Chooser.h"
#include "ChooserFunctionLibrary.h"
#include "NinjaGASPTags.h"
#include "NinjaInteractionTags.h"
#include "Animation/AnimSequenceBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/NinjaCombatComboManagerComponent.h"
#include "Components/NinjaCombatEquipmentAdapterComponent.h"
#include "Components/NinjaCombatManagerComponent.h"
#include "Components/NinjaCombatMotionWarpingComponent.h"
#include "Components/NinjaEquipmentManagerComponent.h"
#include "Components/NinjaInteractionManagerComponent.h"
#include "Components/NinjaInventoryManagerComponent.h"
#include "Components/SphereComponent.h"
#include "Data/NinjaGASPBaseOverlayDataAsset.h"
#include "Data/NinjaGASPPoseOverlayDataAsset.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "GameFramework/NinjaGASPCharacterMovementComponent.h"
#include "Interfaces/EquipmentAnimationInterface.h"
#include "Interfaces/PlayerCameraModeInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Types/EPlayerCameraMode.h"

FName ANinjaGASPCharacter::CombatManagerName = TEXT("CombatManager");
FName ANinjaGASPCharacter::ComboManagerName = TEXT("ComboManager");
FName ANinjaGASPCharacter::MotionWarpingName = TEXT("MotionWarping");
FName ANinjaGASPCharacter::WeaponManagerName = TEXT("WeaponManager");
FName ANinjaGASPCharacter::InventoryManagerName = TEXT("InventoryManager");
FName ANinjaGASPCharacter::EquipmentManagerName = TEXT("EquipmentManager");
FName ANinjaGASPCharacter::InteractionManagerName = TEXT("InteractionManager");
FName ANinjaGASPCharacter::InteractionScanName = TEXT("InteractionScan");

ANinjaGASPCharacter::ANinjaGASPCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNinjaGASPCharacterMovementComponent>(CharacterMovementComponentName))
{
	bIsLocallyControlled = false;
	bJustLanded = false;
	bOverrideEyesViewPointForSightPerception = false;
	SightSenseSourceLocationName = TEXT("head");
	DefaultMeleeEffectLevel = 1.f;
	GameplayCameraConsoleVariable = TEXT("DDCVar.NewGameplayCameraSystem.Enable");
	LandedVelocity = FVector::ZeroVector;
	ActiveSprintAngle = 50.f;
	LandingResetTime = 0.3f;
	TimeToResetTraversalCorrections = 0.2f;
	MovementIntents = FCharacterMovementIntents();
	TraversalActionSummary = FCharacterTraversalActionSummary();
	BaseAnimationOverlay = ECharacterOverlayBase::GASP;
	PoseAnimationOverlay = ECharacterOverlayPose::Default;

	const FName PrimaryMeshTag = Tag_GASP_Component_Mesh_Primary.GetTag().GetTagName(); 
	GetMesh()->ComponentTags.Add(PrimaryMeshTag);
	
	StimuliSenseSources.Add(UAISense_Sight::StaticClass());
	StimuliSenseSources.Add(UAISense_Hearing::StaticClass());
	StimuliSenseSources.Add(UAISense_Damage::StaticClass());

	static FName InteractionScanProfileName(TEXT("InteractionSource"));
	InteractionScan = CreateOptionalDefaultSubobject<USphereComponent>(InteractionScanName);
	InteractionScan->ComponentTags.Add(Tag_Interaction_Component_InteractableScan.GetTag().GetTagName());
	InteractionScan->SetCollisionProfileName(InteractionScanProfileName);
	InteractionScan->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	InteractionScan->SetSphereRadius(200.f);
	InteractionScan->SetupAttachment(GetMesh());
	
	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	CombatManager = CreateDefaultSubobject<UNinjaCombatManagerComponent>(CombatManagerName);
	ComboManager = CreateDefaultSubobject<UNinjaCombatComboManagerComponent>(ComboManagerName);
	MotionWarping = CreateDefaultSubobject<UNinjaCombatMotionWarpingComponent>(MotionWarpingName);
	WeaponManager = CreateDefaultSubobject<UNinjaCombatEquipmentAdapterComponent>(WeaponManagerName);
	InteractionManager = CreateDefaultSubobject<UNinjaInteractionManagerComponent>(InteractionManagerName);
	EquipmentManager = CreateDefaultSubobject<UNinjaEquipmentManagerComponent>(EquipmentManagerName);
	InventoryManager = CreateOptionalDefaultSubobject<UNinjaInventoryManagerComponent>(InventoryManagerName);
}

bool ANinjaGASPCharacter::IsLocallyControlled() const
{
	// Add the cached variable, but keep the original just in case.
	return bIsLocallyControlled || Super::IsLocallyControlled();
}

void ANinjaGASPCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	bIsLocallyControlled = NewController->IsLocalController();
	SetupPlayerCamera();
}

void ANinjaGASPCharacter::UnPossessed()
{
	DisablePlayerCamera();
	ClearHeldObject();
	bIsLocallyControlled = false;
	
	Super::UnPossessed();
}

void ANinjaGASPCharacter::BeginPlay()
{
	Super::BeginPlay();

	ConsolidateBaseAnimationLayer();
	ConsolidatePoseAnimationLayer();
	RegisterStimuliSources();
}

void ANinjaGASPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

void ANinjaGASPCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BaseAnimationOverlay);
	DOREPLIFETIME(ThisClass, PoseAnimationOverlay);
	
	DOREPLIFETIME_CONDITION(ThisClass, MovementIntents, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, TraversalActionSummary, COND_SimulatedOnly);
}

void ANinjaGASPCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (bOverrideEyesViewPointForSightPerception)
	{
		const USkeletalMeshComponent* CharacterMesh = GetMesh();
		if (IsValid(CharacterMesh) && CharacterMesh->DoesSocketExist(SightSenseSourceLocationName))
		{
			const FTransform SocketTransform = CharacterMesh->GetSocketTransform(SightSenseSourceLocationName);
			OutLocation = SocketTransform.GetLocation();
			OutRotation = SocketTransform.GetRotation().Rotator();
			return;
		}
	}
	
	Super::GetActorEyesViewPoint(OutLocation, OutRotation);	
}

void ANinjaGASPCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	const AController* CurrentController = GetController();
	bIsLocallyControlled = IsValid(CurrentController) && CurrentController->IsLocalController();
}

void ANinjaGASPCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	const float GroundSpeedBeforeJump = GetCharacterMovement()->Velocity.Size2D();
	HandleCharacterJumped(GroundSpeedBeforeJump);
}

void ANinjaGASPCharacter::HandleCharacterJumped_Implementation(float GroundSpeedBeforeJump)
{
}

void ANinjaGASPCharacter::Landed(const FHitResult& Hit)
{
	// Store the landed velocity before calling super, since that will broadcast to blueprints,
	// and when that happens, we want to make sure that the property is already updated.
	//
	LandedVelocity = GetCharacterMovement()->Velocity;
	Super::Landed(Hit);
	HandleCharacterLanded(LandedVelocity);
}

void ANinjaGASPCharacter::HandleCharacterLanded_Implementation(const FVector Velocity)
{
	// This is probably being called both on servers and clients. We want to make sure
	// that the flag is set (and reset soon) and also, that whatever velocity was received
	// is set as the Landed Velocity, since simulated clients will provide this.
	//
	bJustLanded = true;
	LandedVelocity = Velocity;

	const FTimerDelegate TimerDelegate = FTimerDelegate::CreateWeakLambda(this, [this](){ bJustLanded = false; }); 
	GetWorld()->GetTimerManager().SetTimer(LandedResetTimerHandle, TimerDelegate, LandingResetTime, false);
}

void ANinjaGASPCharacter::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
	const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float const TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta);
	UnCrouch();
}

void ANinjaGASPCharacter::RegisterStimuliSources()
{
	if (IsValid(StimuliSource) && !StimuliSenseSources.IsEmpty())
	{
		for (const TSubclassOf<UAISense>& Sense : StimuliSenseSources)
		{
			StimuliSource->RegisterForSense(Sense);
		}
	}
}

FCharacterMovementIntents ANinjaGASPCharacter::GetMovementIntents() const
{
	return MovementIntents;
}

void ANinjaGASPCharacter::SetMovementIntents(const FCharacterMovementIntents NewMovementIntents)
{
	// Predict locally. Make sure to replicate via the server as needed.
	if (IsLocallyControlled() || HasAuthority())
	{
		const FCharacterMovementIntents PreviousMovementIntents = MovementIntents;
		MovementIntents = NewMovementIntents;
		OnRep_MovementIntents(PreviousMovementIntents);

		if (!HasAuthority())
		{
			// Local player, without authority, the server has to replicate.
			Server_SetMovementIntents(NewMovementIntents);	
		}
	}
}

void ANinjaGASPCharacter::Server_SetMovementIntents_Implementation(const FCharacterMovementIntents InputStateSettings)
{
	SetMovementIntents(InputStateSettings);
}

bool ANinjaGASPCharacter::Server_SetMovementIntents_Validate(const FCharacterMovementIntents InputStateSettings)
{
	return true;
}

void ANinjaGASPCharacter::OnRep_MovementIntents(const FCharacterMovementIntents OldMovementIntents)
{
	if (MovementIntents.bWantsToStrafe != OldMovementIntents.bWantsToStrafe)
	{
		UCharacterMovementComponent* CharMovement = GetCharacterMovement();
		if (IsValid(CharMovement))
		{
			// If we are strafing, then the CMC should not orient to movement.
			CharMovement->bOrientRotationToMovement = !MovementIntents.bWantsToStrafe;
		}	
	}
}

void ANinjaGASPCharacter::SetWalkingIntent_Implementation(const bool bWantsToWalk)
{
	if (MovementIntents.bWantsToWalk == bWantsToWalk)
	{
		return;
	}
	
	// Technically, the Gameplay Ability should take care of this check, but we want to
	// allow the walking intent to be set from other things like AI tasks, for example.
	//
	const bool bIsSprinting = Execute_IsSprinting(this);
	if (!bIsSprinting)
	{
		FCharacterMovementIntents Intents = MovementIntents;
		Intents.bWantsToWalk = bWantsToWalk;
		SetMovementIntents(Intents);
	}
}

void ANinjaGASPCharacter::SetSprintingIntent_Implementation(const bool bWantsToSprint)
{
	if (MovementIntents.bWantsToSprint == bWantsToSprint)
	{
		return;
	}

	FCharacterMovementIntents Intents = MovementIntents;
	Intents.bWantsToSprint = bWantsToSprint;
	Intents.bWantsToWalk = false;
	SetMovementIntents(Intents);
}

void ANinjaGASPCharacter::SetStrafingIntent_Implementation(const bool bWantsToStrafe)
{
	if (MovementIntents.bWantsToStrafe == bWantsToStrafe)
	{
		return;
	}
	
	// We may or may not implement this, depending on whether the subclass/blueprint is a player or not. 
	if (Implements<UPlayerCameraModeInterface>())
	{
		const EPlayerCameraMode CameraMode = IPlayerCameraModeInterface::Execute_GetCameraMode(this);
		if (CameraMode == EPlayerCameraMode::FirstPerson && bWantsToStrafe == false)
		{
			// In first person mode, we cannot disable strafing.
			return;
		}
	}

	FCharacterMovementIntents Intents = MovementIntents;
	Intents.bWantsToStrafe = bWantsToStrafe;
	SetMovementIntents(Intents);
}

void ANinjaGASPCharacter::SetAimingIntent_Implementation(const bool bWantsToAim)
{
	if (MovementIntents.bWantsToAim == bWantsToAim)
	{
		return;
	}

	FCharacterMovementIntents Intents = MovementIntents;
	Intents.bWantsToAim = bWantsToAim;
	SetMovementIntents(Intents);
}

bool ANinjaGASPCharacter::IsActivelyRunning_Implementation() const
{
	const bool bWantsToWalk = GetMovementIntents().bWantsToWalk;
	const bool bWantsToSprint = GetMovementIntents().bWantsToSprint;
	if (!bWantsToWalk || !bWantsToSprint)
	{
		// An overriding intent is set.
		return false;
	}

	const UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!IsValid(CMC))
	{
		// We need a CMC to check this here. Other implementations requires overriding.
		return false;
	}

	static constexpr float Tolerance = 0.001f;
	return !UKismetMathLibrary::Vector_IsNearlyZero(CMC->Velocity, Tolerance);	
}

bool ANinjaGASPCharacter::IsActivelySprinting_Implementation() const
{
	const bool bWantsToSprint = GetMovementIntents().bWantsToSprint;
	if (!bWantsToSprint)
	{
		// No intent. We should not be sprinting.
		return false;
	}

	const UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (IsValid(CMC) && CMC->bOrientRotationToMovement)
	{
		// We have the intent and we are moving forward. If we have velocity,
		// then we can consider that the character is actively sprinting.
		return CMC->Velocity.Size2D() > 0.f;
	}

	if (!IsValid(CMC))
	{
		// Sanity check as we should always have a CMC here (and we need one for this check).
		// But if we don't have one for some reason, then skip the logic altogether.
		return false;	
	}
	
	const FVector Acceleration = IsLocallyControlled() ? CMC->GetPendingInputVector() : CMC->GetCurrentAcceleration();
	if (Acceleration.IsNearlyZero())
	{
		// No meaningful acceleration.
		return false;
	}
	
	const FRotator GroundRotation = UKismetMathLibrary::Conv_VectorToRotator(Acceleration);
	const FRotator ActualRotation = GetActorRotation();
	const FRotator MovementDirection = UKismetMathLibrary::NormalizedDeltaRotator(ActualRotation, GroundRotation);

	const float MovementDirectionYaw = FMath::Abs(MovementDirection.Yaw);
	return MovementDirectionYaw < ActiveSprintAngle
		&& CMC->Velocity.Size2D() > 0.f;
}

bool ANinjaGASPCharacter::ShouldUseGameplayCameras() const
{
	return UKismetSystemLibrary::GetConsoleVariableBoolValue(GameplayCameraConsoleVariable);
}

FCharacterTraversalActionSummary ANinjaGASPCharacter::GetActiveTraversalAction() const
{
	return TraversalActionSummary;
}

void ANinjaGASPCharacter::RegisterTraversalAction(const ECharacterTraversalAction ActionType, UPrimitiveComponent* Target)
{
	if (TraversalActionSummary.Action != ECharacterTraversalAction::None)
	{
		// Traversal action already happening!
		return; 
	}

	if (ActionType == ECharacterTraversalAction::None || !IsValid(Target))
	{
		// Invalid input data.
		return;
	}

	// Predict locally. Make sure to replicate via the server as needed.
	if (IsLocallyControlled() || HasAuthority())
	{
		const FCharacterTraversalActionSummary OldSummary = TraversalActionSummary;
		TraversalActionSummary.Action = ActionType;
		TraversalActionSummary.Target = Target;
		OnRep_TraversalAction(OldSummary);

		if (!HasAuthority())
		{
			Server_RegisterTraversalAction(ActionType, Target);
		}
	}
}

void ANinjaGASPCharacter::Server_RegisterTraversalAction_Implementation(const ECharacterTraversalAction ActionType, UPrimitiveComponent* Target)
{
	RegisterTraversalAction(ActionType, Target);
}

bool ANinjaGASPCharacter::Server_RegisterTraversalAction_Validate(const ECharacterTraversalAction ActionType, UPrimitiveComponent* Target)
{
	return true;
}

void ANinjaGASPCharacter::ClearTraversalAction()
{
	if (TraversalActionSummary.Action == ECharacterTraversalAction::None)
	{
		// No traversal action happening.
		return; 
	}

	// Predict locally. Make sure to replicate via the server as needed.
	if (IsLocallyControlled() || HasAuthority())
	{
		const FCharacterTraversalActionSummary OldSummary = TraversalActionSummary; 
		TraversalActionSummary.Action = ECharacterTraversalAction::None;
		TraversalActionSummary.Target = nullptr;
		OnRep_TraversalAction(OldSummary);

		if (!HasAuthority())
		{
			Server_ClearTraversalAction();
		}
	}
}

void ANinjaGASPCharacter::Server_ClearTraversalAction_Implementation()
{
	ClearTraversalAction();
}

bool ANinjaGASPCharacter::Server_ClearTraversalAction_Validate()
{
	return true;
}

void ANinjaGASPCharacter::OnRep_TraversalAction(const FCharacterTraversalActionSummary OldTraversalActionSummary)
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (IsValid(CMC))
	{
		const bool bOldHadTraversal = OldTraversalActionSummary.Action != ECharacterTraversalAction::None;
		const bool bNewHasTraversal = TraversalActionSummary.Action != ECharacterTraversalAction::None;

		if (!bOldHadTraversal && bNewHasTraversal)
		{
			CMC->bIgnoreClientMovementErrorChecksAndCorrection = true;
			CMC->bServerAcceptClientAuthoritativePosition = true;
			CMC->SetMovementMode(MOVE_Flying);
		}

		else if (bOldHadTraversal && !bNewHasTraversal)
		{
			// Reset correction flags after a short delay.
			const FTimerDelegate TimerDelegate = FTimerDelegate::CreateWeakLambda(this, [CMC]()
			{
				if (IsValid(CMC))
				{
					CMC->bIgnoreClientMovementErrorChecksAndCorrection = false;
					CMC->bServerAcceptClientAuthoritativePosition = false;
				}
			});

			GetWorld()->GetTimerManager().SetTimer(TraversalCorrectionTimerHandle, TimerDelegate, TimeToResetTraversalCorrections, false);

			// Decide which movement mode we go back to, based on the *previous* action.
			EMovementMode NewMovementMode = MOVE_Walking;
			if (OldTraversalActionSummary.Action == ECharacterTraversalAction::Vault)
			{
				NewMovementMode = MOVE_Falling;
			}

			CMC->SetMovementMode(NewMovementMode);
		}
	}

	// If we previously had a target, and either traversal ended or the target changed, then stop ignoring collisions.
	if (OldTraversalActionSummary.Target.IsValid() && OldTraversalActionSummary.Target->IsValidLowLevelFast())
	{
		const bool bNewHasTraversal = TraversalActionSummary.Action != ECharacterTraversalAction::None;
		const bool bTargetChanged = TraversalActionSummary.Target.Get() != OldTraversalActionSummary.Target.Get();

		if (!bNewHasTraversal || bTargetChanged)
		{
			static constexpr bool bShouldIgnore = false;
			GetCapsuleComponent()->IgnoreComponentWhenMoving(OldTraversalActionSummary.Target.Get(), bShouldIgnore);
		}
	}

	// If we now have an active traversal and a valid target, ignore it while moving.
	if (TraversalActionSummary.Action != ECharacterTraversalAction::None && TraversalActionSummary.Target.IsValid() && TraversalActionSummary.Target->IsValidLowLevelFast())
	{
		static constexpr bool bShouldIgnore = true;
		GetCapsuleComponent()->IgnoreComponentWhenMoving(TraversalActionSummary.Target.Get(),bShouldIgnore);
	}
}

void ANinjaGASPCharacter::SetBaseAnimationOverlay(const ECharacterOverlayBase NewBase)
{
	if (BaseAnimationOverlay == NewBase)
	{
		return; 
	}

	// We want to support local prediction, but the server will still be authoritative.
	// This means setting this on the server will still set this on the local client.
	//
	if (IsLocallyControlled() || HasAuthority())
	{
		BaseAnimationOverlay = NewBase;
		OnRep_BaseAnimationOverlay();

		if (!HasAuthority())
		{
			Server_SetBaseAnimationOverlay(NewBase);
		}
	}
}

void ANinjaGASPCharacter::Server_SetBaseAnimationOverlay_Implementation(ECharacterOverlayBase NewBase)
{
	SetBaseAnimationOverlay(NewBase);
}

bool ANinjaGASPCharacter::Server_SetBaseAnimationOverlay_Validate(ECharacterOverlayBase NewBase)
{
	return true;
}

void ANinjaGASPCharacter::OnRep_BaseAnimationOverlay()
{
	ConsolidateBaseAnimationLayer();
}

void ANinjaGASPCharacter::ConsolidateBaseAnimationLayer()
{
	// We track this because we support both prediction and server-replication to local client.
	// But we don't want to apply the layer twice, when prediction + replication happens.
	//
	if (LastProcessedBaseOverlay != BaseAnimationOverlay)
	{
		LastProcessedBaseOverlay = BaseAnimationOverlay;
		HandleBaseAnimationOverlayChanged(BaseAnimationOverlay);
		OnBaseAnimationOverlayChanged.Broadcast(BaseAnimationOverlay);
	}	
}

void ANinjaGASPCharacter::HandleBaseAnimationOverlayChanged_Implementation(ECharacterOverlayBase CurrentBase)
{
	if (IsValid(BaseOverlayChooserTable))
	{
		FChooserEvaluationContext ChooserEvaluationContext = UChooserFunctionLibrary::MakeChooserEvaluationContext();
		ChooserEvaluationContext.AddObjectParam(this);
		
		const FInstancedStruct ChooserInstance = UChooserFunctionLibrary::MakeEvaluateChooser(BaseOverlayChooserTable);
		const UObject* ChooserObject = UChooserFunctionLibrary::EvaluateObjectChooserBase(ChooserEvaluationContext, ChooserInstance, UNinjaGASPBaseOverlayDataAsset::StaticClass());
		const UNinjaGASPBaseOverlayDataAsset* BaseOverlayData = Cast<UNinjaGASPBaseOverlayDataAsset>(ChooserObject); 
		
		if (IsValid(BaseOverlayData))
		{
			const TSubclassOf<UAnimInstance> LayerClass = BaseOverlayData->OverlayAnimationClass;
			LinkOverlayAnimationLayer(LayerClass);
			
			const FCharacterOverlayActivationAnimation Params = BaseOverlayData->ActivationMontage;
			PlaySlotAnimationAsDynamicMontage(Params);
		}
	}
}

void ANinjaGASPCharacter::SetPoseAnimationOverlay(const ECharacterOverlayPose NewPose)
{
	if (PoseAnimationOverlay == NewPose)
	{
		return; 
	}

	// We want to support local prediction, but the server will still be authoritative.
	// This means setting this on the server will still set this on the local client.
	//
	if (IsLocallyControlled() || HasAuthority())
	{
		PoseAnimationOverlay = NewPose;
		OnRep_PoseAnimationOverlay();

		if (!HasAuthority())
		{
			Server_SetPoseAnimationOverlay(NewPose);
		}
	}	
}

void ANinjaGASPCharacter::Server_SetPoseAnimationOverlay_Implementation(ECharacterOverlayPose NewPose)
{
	SetPoseAnimationOverlay(NewPose);
}

bool ANinjaGASPCharacter::Server_SetPoseAnimationOverlay_Validate(ECharacterOverlayPose NewPose)
{
	return true;
}

void ANinjaGASPCharacter::OnRep_PoseAnimationOverlay()
{
	ConsolidatePoseAnimationLayer();
}

void ANinjaGASPCharacter::ConsolidatePoseAnimationLayer()
{
	// We track this because we support both prediction and server-replication to local client.
	// But we don't want to apply the layer twice, when prediction + replication happens.
	//
	if (LastProcessedPoseOverlay != PoseAnimationOverlay)
	{
		LastProcessedPoseOverlay = PoseAnimationOverlay;
		HandlePoseAnimationOverlayChanged(PoseAnimationOverlay);
		OnPoseAnimationOverlayChanged.Broadcast(PoseAnimationOverlay);
	}
}

void ANinjaGASPCharacter::HandlePoseAnimationOverlayChanged_Implementation(ECharacterOverlayPose CurrentPose)
{
	if (IsValid(PoseOverlayChooserTable))
	{
		FChooserEvaluationContext ChooserEvaluationContext = UChooserFunctionLibrary::MakeChooserEvaluationContext();
		ChooserEvaluationContext.AddObjectParam(this);
		
		const FInstancedStruct ChooserInstance = UChooserFunctionLibrary::MakeEvaluateChooser(PoseOverlayChooserTable);
		const UObject* ChooserObject = UChooserFunctionLibrary::EvaluateObjectChooserBase(ChooserEvaluationContext, ChooserInstance, UNinjaGASPPoseOverlayDataAsset::StaticClass());
		const UNinjaGASPPoseOverlayDataAsset* PoseOverlayData = Cast<UNinjaGASPPoseOverlayDataAsset>(ChooserObject); 
		
		if (IsValid(PoseOverlayData))
		{
			const TSubclassOf<UAnimInstance> LayerClass = PoseOverlayData->OverlayAnimationClass;
			LinkOverlayAnimationLayer(LayerClass);
			
			const FCharacterOverlayActivationAnimation Params = PoseOverlayData->ActivationMontage;
			PlaySlotAnimationAsDynamicMontage(Params);

			ClearHeldObject();
			AttachPoseObjectToHand(PoseOverlayData);
		}
	}
}

void ANinjaGASPCharacter::LinkOverlayAnimationLayer(const TSubclassOf<UAnimInstance> LayerClass) const
{
	UAnimInstance* TargetAnimInstance = Execute_GetCombatAnimInstance(this);
	if (IsValid(TargetAnimInstance) && LayerClass)
	{
		TargetAnimInstance->LinkAnimClassLayers(LayerClass);
	}
}

void ANinjaGASPCharacter::PlaySlotAnimationAsDynamicMontage(const FCharacterOverlayActivationAnimation& Params) const
{
	UAnimInstance* TargetAnimInstance = Execute_GetCombatAnimInstance(this);
	if (IsValid(TargetAnimInstance))
	{
		UAnimSequenceBase* ActivationAnimation = Params.DynamicAnimation;
		if (IsValid(ActivationAnimation))
		{
			TargetAnimInstance->PlaySlotAnimationAsDynamicMontage(ActivationAnimation, Params.SlotName, Params.BlendInTime,
				Params.BlendOutTime, Params.PlayRate, Params.LoopCount, Params.BlendOutTriggerTime, Params.StartTime);
		}		
	}
}

void ANinjaGASPCharacter::AttachPoseObjectToHand_Implementation(const UNinjaGASPPoseOverlayDataAsset* PoseData)
{
	if (!IsValid(PoseData) || PoseData->OverlayItemType == EPoseOverlayItemType::None)
	{
		return;
	}

	USkeletalMeshComponent* RootMesh = Execute_GetCombatMesh(this);
	if (!IsValid(RootMesh))
	{
		return;
	}

	UMeshComponent* NewComponent = nullptr;
	
	if (PoseData->OverlayItemType == EPoseOverlayItemType::StaticMesh)
	{
		UStaticMeshComponent* NewStaticMesh = NewObject<UStaticMeshComponent>(this);
		NewStaticMesh->ComponentTags.Add(Tag_GASP_Component_Mesh_Overlay_Static.GetTag().GetTagName());
		NewStaticMesh->SetStaticMesh(PoseData->StaticMesh);
		NewComponent = NewStaticMesh; 
	}
	else if (PoseData->OverlayItemType == EPoseOverlayItemType::SkeletalMesh)
	{
		USkeletalMeshComponent* NewSkeletalMesh = NewObject<USkeletalMeshComponent>(this);
		NewSkeletalMesh->ComponentTags.Add(Tag_GASP_Component_Mesh_Overlay_Skeletal.GetTag().GetTagName());
		NewSkeletalMesh->SetSkeletalMesh(PoseData->SkeletalMesh, true);

		if (PoseData->SkeletalMeshAnimClass)
		{
			NewSkeletalMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			NewSkeletalMesh->SetAnimInstanceClass(PoseData->SkeletalMeshAnimClass);
			SetOwnerToPoseOverlayAnimationInstance(NewSkeletalMesh);
		}
		
		NewComponent = NewSkeletalMesh; 
	}

	if (IsValid(NewComponent))
	{
		NewComponent->AttachToComponent(RootMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, PoseData->SocketName);
		NewComponent->RegisterComponent();
		NewComponent->Activate();
		FinishAndRegisterComponent(NewComponent);
	}
}

void ANinjaGASPCharacter::SetOwnerToPoseOverlayAnimationInstance(USkeletalMeshComponent* PoseItemMesh)
{
	if (IsValid(PoseItemMesh))
	{
		UAnimInstance* NewAnimInstance = PoseItemMesh->GetAnimInstance();
		if (NewAnimInstance)
		{
			if (NewAnimInstance->Implements<UEquipmentAnimationInterface>())
			{
				// Realistically, this should be handled by the equipment instance.
				IEquipmentAnimationInterface::Execute_SetInventoryAvatar(NewAnimInstance, this);
			}
		}
		else
		{
			const FTimerDelegate EquipmentDataDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::SetOwnerToPoseOverlayAnimationInstance, PoseItemMesh);
			GetWorld()->GetTimerManager().SetTimerForNextTick(EquipmentDataDelegate);
		}
	}
}

void ANinjaGASPCharacter::ClearHeldObject_Implementation()
{
	auto SeekAndDestroy = [&](const FName TagName)
	{
		TArray<UActorComponent*> Components = GetComponentsByTag(UActorComponent::StaticClass(), TagName);
		for (UActorComponent* Component : Components)
		{
			Component->UnregisterComponent();
			Component->DestroyComponent();
		}
	};
	
	const FName SkeletalMeshTag = Tag_GASP_Component_Mesh_Overlay_Skeletal.GetTag().GetTagName();
	SeekAndDestroy(SkeletalMeshTag);

	const FName StaticMeshTag = Tag_GASP_Component_Mesh_Overlay_Static.GetTag().GetTagName();
	SeekAndDestroy(StaticMeshTag);
}

void ANinjaGASPCharacter::SetupPlayerCamera_Implementation()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (HasAuthority() && !IsLocallyControlled())
	{
		Client_SetupPlayerCamera();
		return;
	}

	if (IsLocallyControlled())
	{
		if (ShouldUseGameplayCameras())
		{
			UGameplayCameraComponent* GameplayCamera = FindComponentByClass<UGameplayCameraComponent>();
			if (IsValid(GameplayCamera))
			{
				GameplayCamera->ActivateCameraForPlayerController(PlayerController);
				OnPlayerCameraActivated(PlayerController);
			}
		}
		else if (UCameraComponent* CameraComponent = FindComponentByClass<UCameraComponent>())
		{
			CameraComponent->Activate();
			OnPlayerCameraActivated(PlayerController);
		}
	}
}

void ANinjaGASPCharacter::Client_SetupPlayerCamera_Implementation()
{
	SetupPlayerCamera();
}

void ANinjaGASPCharacter::DisablePlayerCamera_Implementation()
{
	if (HasAuthority() && !IsLocallyControlled())
	{
		Client_SetupPlayerCamera();
		return;
	}

	if (IsLocallyControlled())
	{
		if (ShouldUseGameplayCameras())
		{
			if (UGameplayCameraComponent* GameplayCamera = FindComponentByClass<UGameplayCameraComponent>())
			{
				OnPlayerCameraDeactivated();
				GameplayCamera->DeactivateCamera(true);
			}
		}
		else if (UCameraComponent* CameraComponent = FindComponentByClass<UCameraComponent>())
		{
			OnPlayerCameraDeactivated();
			CameraComponent->Deactivate();
		}
	}
}

void ANinjaGASPCharacter::Client_DisablePlayerCamera_Implementation()
{
	DisablePlayerCamera();
}

UNinjaCombatManagerComponent* ANinjaGASPCharacter::GetCombatManager_Implementation() const
{
	return CombatManager;
}

USkeletalMeshComponent* ANinjaGASPCharacter::GetCombatMesh_Implementation() const
{
	return GetMesh();
}

UAnimInstance* ANinjaGASPCharacter::GetCombatAnimInstance_Implementation() const
{
	// In GASP, this is very important!
	//
	// We want to always return the anim instance that is driving the animation, and not the retarget proxy. 
	// This means our Anim Instance always comes from the "main mesh" even when we change the combat mesh!
	//
	return GetMesh()->GetAnimInstance();
}

UActorComponent* ANinjaGASPCharacter::GetComboManagerComponent_Implementation() const
{
	return ComboManager;
}

UActorComponent* ANinjaGASPCharacter::GetMotionWarpingComponent_Implementation() const
{
	return MotionWarping;
}

UActorComponent* ANinjaGASPCharacter::GetWeaponManagerComponent_Implementation() const
{
	return WeaponManager;
}

UMeshComponent* ANinjaGASPCharacter::GetMeleeMesh_Implementation() const
{
	return Execute_GetCombatMesh(this);
}

TSubclassOf<UGameplayEffect> ANinjaGASPCharacter::GetHitEffectClass_Implementation() const
{
	return DefaultMeleeEffectClass;
}

float ANinjaGASPCharacter::GetHitEffectLevel_Implementation() const
{
	return DefaultMeleeEffectLevel;
}

UMeshComponent* ANinjaGASPCharacter::GetProjectileSourceMesh_Implementation(FName SocketName) const
{
	return Execute_GetCombatMesh(this);
}

UNinjaInventoryManagerComponent* ANinjaGASPCharacter::GetInventoryManager_Implementation() const
{
	return InventoryManager;
}

UNinjaEquipmentManagerComponent* ANinjaGASPCharacter::GetEquipmentManager_Implementation() const
{
	return EquipmentManager;
}
