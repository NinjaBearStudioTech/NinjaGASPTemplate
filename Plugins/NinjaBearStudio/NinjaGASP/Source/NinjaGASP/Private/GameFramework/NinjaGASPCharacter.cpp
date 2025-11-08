// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPCharacter.h"

#include "NinjaGASPTags.h"
#include "Camera/CameraComponent.h"
#include "Components/NinjaCombatComboManagerComponent.h"
#include "Components/NinjaCombatEquipmentAdapterComponent.h"
#include "Components/NinjaCombatManagerComponent.h"
#include "Components/NinjaCombatMotionWarpingComponent.h"
#include "Components/NinjaEquipmentManagerComponent.h"
#include "Components/NinjaInventoryManagerComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "GameFramework/NinjaGASPCharacterMovementComponent.h"
#include "Interfaces/PlayerCameraModeInterface.h"
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
	LandingResetTime = 0.3f;
	MovementIntents = FCharacterMovementIntents();

	const FName PrimaryMeshTag = Tag_GASP_Component_Mesh_Primary.GetTag().GetTagName(); 
	GetMesh()->ComponentTags.Add(PrimaryMeshTag);
	
	StimuliSenseSources.Add(UAISense_Sight::StaticClass());
	StimuliSenseSources.Add(UAISense_Hearing::StaticClass());
	StimuliSenseSources.Add(UAISense_Damage::StaticClass());
	
	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	CombatManager = CreateDefaultSubobject<UNinjaCombatManagerComponent>(CombatManagerName);
	ComboManager = CreateDefaultSubobject<UNinjaCombatComboManagerComponent>(ComboManagerName);
	MotionWarping = CreateDefaultSubobject<UNinjaCombatMotionWarpingComponent>(MotionWarpingName);
	WeaponManager = CreateDefaultSubobject<UNinjaCombatEquipmentAdapterComponent>(WeaponManagerName);
	InventoryManager = CreateOptionalDefaultSubobject<UNinjaInventoryManagerComponent>(InventoryManagerName);
	EquipmentManager = CreateDefaultSubobject<UNinjaEquipmentManagerComponent>(EquipmentManagerName);
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
	DOREPLIFETIME_CONDITION(ThisClass, MovementIntents, COND_SkipOwner);
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
		const FCharacterMovementIntents Intents = FCharacterMovementIntents::ForWalking(bWantsToWalk);
		SetMovementIntents(Intents);
	}
}

void ANinjaGASPCharacter::SetSprintingIntent_Implementation(const bool bWantsToSprint)
{
	if (MovementIntents.bWantsToSprint == bWantsToSprint)
	{
		return;
	}

	const FCharacterMovementIntents Intents = FCharacterMovementIntents::ForSprinting(bWantsToSprint);
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

	const FCharacterMovementIntents Intents = FCharacterMovementIntents::ForStrafing(bWantsToStrafe);
	SetMovementIntents(Intents);
}

void ANinjaGASPCharacter::SetAimingIntent_Implementation(const bool bWantsToAim)
{
	if (MovementIntents.bWantsToAim == bWantsToAim)
	{
		return;
	}

	const FCharacterMovementIntents Intents = FCharacterMovementIntents::ForAiming(bWantsToAim);
	SetMovementIntents(Intents);
}

bool ANinjaGASPCharacter::ShouldUseGameplayCameras() const
{
	return UKismetSystemLibrary::GetConsoleVariableBoolValue(GameplayCameraConsoleVariable);
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

