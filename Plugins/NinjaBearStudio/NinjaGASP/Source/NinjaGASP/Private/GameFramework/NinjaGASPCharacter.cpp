// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPCharacter.h"

#include "AIController.h"
#include "Camera/CameraComponent.h"
#include "Components/NinjaCombatComboManagerComponent.h"
#include "Components/NinjaCombatManagerComponent.h"
#include "Components/NinjaCombatMotionWarpingComponent.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "GameFramework/NinjaGASPCharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

FName ANinjaGASPCharacter::CombatManagerName = TEXT("CombatManager");
FName ANinjaGASPCharacter::ComboManagerName = TEXT("ComboManager");
FName ANinjaGASPCharacter::MotionWarpingName = TEXT("MotionWarping");

ANinjaGASPCharacter::ANinjaGASPCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNinjaGASPCharacterMovementComponent>(CharacterMovementComponentName))
{
	bIsLocallyControlled = false;
	bOverrideEyesViewPointForSightPerception = false;
	SightSenseSourceLocationName = TEXT("head");
	DefaultMeleeEffectLevel = 1.f;
	GameplayCameraConsoleVariable = TEXT("DDCVar.NewGameplayCameraSystem.Enable");
	
	StimuliSenseSources.Add(UAISense_Sight::StaticClass());
	StimuliSenseSources.Add(UAISense_Hearing::StaticClass());
	StimuliSenseSources.Add(UAISense_Damage::StaticClass());
	
	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	CombatManager = CreateDefaultSubobject<UNinjaCombatManagerComponent>(CombatManagerName);
	ComboManager = CreateDefaultSubobject<UNinjaCombatComboManagerComponent>(ComboManagerName);
	MotionWarping = CreateDefaultSubobject<UNinjaCombatMotionWarpingComponent>(MotionWarpingName);
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
	SetupCharacterMovement();
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

void ANinjaGASPCharacter::SetupCharacterMovement()
{
	const AController* MyController = GetController();
	if (IsValid(MyController))
	{
		if (MyController->IsA<AAIController>())
		{
			SetupBotMovement();	
		}
		else
		{
			SetupPlayerMovement();
		}
	}
}

void ANinjaGASPCharacter::SetupBotMovement()
{
	static constexpr bool bWantsToStrafe = false; 
	Execute_SetStrafingIntent(this, bWantsToStrafe);

	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (IsValid(CharMovement))
	{
		CharMovement->bOrientRotationToMovement = true;
	}
}

void ANinjaGASPCharacter::SetupPlayerMovement()
{
	static constexpr bool bWantsToStrafe = true; 
	Execute_SetStrafingIntent(this, bWantsToStrafe);

	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (IsValid(CharMovement))
	{
		CharMovement->bOrientRotationToMovement = false;
	}	
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
	// In GASP, this is very important. We want to always return the anim instance that
	// is driving the animation, and not the retarget proxy. This means our Anim Instance
	// always comes from the "main mesh" even when we change the combat mesh!
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
