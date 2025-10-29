// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NinjaGASCharacter.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"
#include "Interfaces/CombatMeleeInterface.h"
#include "Interfaces/CombatSystemInterface.h"
#include "Interfaces/PreMovementComponentTickInterface.h"
#include "Interfaces/TraversalMovementInputInterface.h"
#include "NinjaGASPCharacter.generated.h"

class UAISense;
class UNinjaCombatMotionWarpingComponent;
class UAIPerceptionStimuliSourceComponent;
class UNinjaCombatManagerComponent;
class UNinjaCombatComboManagerComponent;

/**
 * Base character, with functionality that can be shared between players and AI.
 *
 * Provides the boilerplate code necessary for the Gameplay Ability System, but Player Classes
 * using a Player State might need to perform certain extensions to disable that (this framework
 * provides an appropriate class to that end)
 */
UCLASS(Abstract)
class NINJAGASP_API ANinjaGASPCharacter : public ANinjaGASCharacter, public IPreMovementComponentTickInterface,
	public IAdvancedCharacterMovementInterface, public ITraversalMovementInputInterface, public ICombatSystemInterface, 
	public ICombatMeleeInterface
{
	
	GENERATED_BODY()

public:

	static FName CombatManagerName;
	static FName ComboManagerName;
	static FName MotionWarpingName;
	
	ANinjaGASPCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Pawn implementation
	virtual void OnPossess(APawn *InPawn);
	virtual void BeginPlay() override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
	// -- End Pawn implementation

	// -- Begin Combat System implementation
	virtual UNinjaCombatManagerComponent* GetCombatManager_Implementation() const override;
	virtual USkeletalMeshComponent* GetCombatMesh_Implementation() const override;
	virtual UAnimInstance* GetCombatAnimInstance_Implementation() const override;
	virtual UActorComponent* GetComboManagerComponent_Implementation() const override;
	virtual UActorComponent* GetMotionWarpingComponent_Implementation() const override;
	// -- End Combat System implementation

	// -- Begin Melee Mesh implementation
	virtual UMeshComponent* GetMeleeMesh_Implementation() const override;
	virtual TSubclassOf<UGameplayEffect> GetHitEffectClass_Implementation() const override;
	virtual float GetHitEffectLevel_Implementation() const override;
	// -- End Melee Mesh implementation
	
protected:

	/** All stimuli sources registered with this character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TArray<TSubclassOf<UAISense>> StimuliSenseSources;

	/** If enabled, allows overriding the source for Sight perception to this bone or socket. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (InlineEditConditionToggle))
	bool bOverrideEyesViewPointForSightPerception;

	/** Bone or socket used as the source (location and rotation) used by the Sight sense as "eye location". */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (EditCondition = bOverrideEyesViewPointForSightPerception))
	FName SightSenseSourceLocationName;
	
	/**
	 * Default gameplay effect used when melee damage is applied.
	 *
	 * Can be adjusted by curves/calculations/modifiers, or fully overwritten by the melee
	 * ability itself (or any melee scan executed by it).
	 *
	 * You can also provide dynamic values as needed by overwriting "GetHitEffectClass",
	 * from the Combat Melee Interface, implemented by this character.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat")
	TSubclassOf<UGameplayEffect> DefaultMeleeEffectClass;

	/**
	 * Level for the default Melee Effect applied.
	 *
	 * This determines which curve values are retrieved (if the effect uses curves). Values
	 * equal or lower than zero will be discarded. In which case, the ability level will be
	 * used instead.
	 *
	 * You can also provide dynamic values as needed by overwriting "GetHitEffectLevel",
	 * from the Combat Melee Interface, implemented by this character.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Combat", meta = (UIMin = 0.f, ClampMin = 0.f))
	float DefaultMeleeEffectLevel;

	/** Registers all stimuli sources when we have a valid world. */
	void RegisterStimuliSources();
	
	/** Configures movement-related features for an AI controlled pawn. */
	void SetupBotMovement();

	/** Configures movement-related features for a Player controlled pawn. */
	void SetupPlayerMovement();

private:

	/** Registers all stimuli sources for this character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;

	/** Central component for the combat system (also works as damage, defense and other core components). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNinjaCombatManagerComponent> CombatManager;

	/** Manages attacks triggered in sequence. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNinjaCombatComboManagerComponent> ComboManager;

	/** Motion warping component used by both GASP and the combat system. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNinjaCombatMotionWarpingComponent> MotionWarping;
	
};
