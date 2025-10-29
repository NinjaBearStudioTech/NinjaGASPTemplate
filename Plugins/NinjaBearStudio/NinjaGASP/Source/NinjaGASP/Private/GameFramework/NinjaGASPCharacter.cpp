// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPCharacter.h"

#include "AIController.h"
#include "Components/NinjaCombatComboManagerComponent.h"
#include "Components/NinjaCombatManagerComponent.h"
#include "Components/NinjaCombatMotionWarpingComponent.h"
#include "GameFramework/NinjaGASPCharacterMovementComponent.h"
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
	bOverrideEyesViewPointForSightPerception = false;
	SightSenseSourceLocationName = TEXT("head");
	DefaultMeleeEffectLevel = 1.f;
	
	StimuliSenseSources.Add(UAISense_Sight::StaticClass());
	StimuliSenseSources.Add(UAISense_Hearing::StaticClass());
	StimuliSenseSources.Add(UAISense_Damage::StaticClass());
	
	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	CombatManager = CreateDefaultSubobject<UNinjaCombatManagerComponent>(CombatManagerName);
	ComboManager = CreateDefaultSubobject<UNinjaCombatComboManagerComponent>(ComboManagerName);
	MotionWarping = CreateDefaultSubobject<UNinjaCombatMotionWarpingComponent>(MotionWarpingName);
}

void ANinjaGASPCharacter::OnPossess(APawn* InPawn)
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
