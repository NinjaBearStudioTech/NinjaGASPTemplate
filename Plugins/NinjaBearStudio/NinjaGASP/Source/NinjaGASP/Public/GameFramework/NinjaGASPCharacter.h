// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NinjaGASCharacter.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"
#include "Interfaces/CombatMeleeInterface.h"
#include "Interfaces/CombatRangedInterface.h"
#include "Interfaces/CombatSystemInterface.h"
#include "Interfaces/EquipmentSystemInterface.h"
#include "Interfaces/InventorySystemInterface.h"
#include "Interfaces/PreMovementComponentTickInterface.h"
#include "Interfaces/TraversalMovementInputInterface.h"
#include "NinjaGASPCharacter.generated.h"

class UAISense;
class UGameplayCameraComponent;
class UNinjaCombatMotionWarpingComponent;
class UAIPerceptionStimuliSourceComponent;
class UNinjaCombatManagerComponent;
class UNinjaCombatComboManagerComponent;
class UNinjaCombatBaseWeaponManagerComponent;
class UNinjaInventoryManagerComponent;
class UNinjaEquipmentManagerComponent;

/**
 * Base character, with functionality that can be shared between players and AI.
 */
UCLASS(Abstract)
class NINJAGASP_API ANinjaGASPCharacter : public ANinjaGASCharacter, public IPreMovementComponentTickInterface,
	public IAdvancedCharacterMovementInterface, public ITraversalMovementInputInterface, public ICombatSystemInterface, 
	public ICombatMeleeInterface, public ICombatRangedInterface, public IInventorySystemInterface, public IEquipmentSystemInterface
{
	
	GENERATED_BODY()

public:

	ANinjaGASPCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Pawn implementation
	virtual bool IsLocallyControlled() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void BeginPlay() override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
	virtual void OnRep_Controller() override;
	// -- End Pawn implementation

	// -- Begin Combat/Melee/Ranged implementation
	virtual UNinjaCombatManagerComponent* GetCombatManager_Implementation() const override;
	virtual USkeletalMeshComponent* GetCombatMesh_Implementation() const override;
	virtual UAnimInstance* GetCombatAnimInstance_Implementation() const override;
	virtual UActorComponent* GetComboManagerComponent_Implementation() const override;
	virtual UActorComponent* GetMotionWarpingComponent_Implementation() const override;
	virtual UActorComponent* GetWeaponManagerComponent_Implementation() const override;
	virtual UMeshComponent* GetMeleeMesh_Implementation() const override;
	virtual TSubclassOf<UGameplayEffect> GetHitEffectClass_Implementation() const override;
	virtual float GetHitEffectLevel_Implementation() const override;
	virtual UMeshComponent* GetProjectileSourceMesh_Implementation(FName SocketName) const override;

	// -- Begin Inventory/Equipment implementation
	virtual UNinjaInventoryManagerComponent* GetInventoryManager_Implementation() const override;
	virtual UNinjaEquipmentManagerComponent* GetEquipmentManager_Implementation() const override;
	// -- End Inventory/Equipment implementation
	
	/**
	 * Checks if the character should use Gameplay Cameras, based on the cvar.
	 */
	UFUNCTION(BlueprintPure, Category = "GASP|Character")
	bool ShouldUseGameplayCameras() const;
	
	/**
	 * Clears any object directly held by the character.
	 *
	 * Usually, this is mostly related to the default GASPALS object attachment and this function
	 * is simply exposing that. But as you get more and more into the Inventory system integration,
	 * this should be just "prototype code".
	 *
	 * Eventually, when the entire functionality is migrated to the inventory integration, including
	 * a selection wheel for these default GASPALS items, this functionality might be removed.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GASP|Character", meta = (ForceAsFunction))
	void ClearHeldObject();
	virtual void ClearHeldObject_Implementation() {  }
	
protected:

	static FName CombatManagerName;
	static FName ComboManagerName;
	static FName MotionWarpingName;
	static FName WeaponManagerName;
	static FName InventoryManagerName;
	static FName EquipmentManagerName;
	
	/** All stimuli sources registered with this character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|AI")
	TArray<TSubclassOf<UAISense>> StimuliSenseSources;

	/** If enabled, allows overriding the source for Sight perception to this bone or socket. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|AI", meta = (InlineEditConditionToggle))
	bool bOverrideEyesViewPointForSightPerception;

	/** Bone or socket used as the source (location and rotation) used by the Sight sense as "eye location". */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|AI", meta = (EditCondition = bOverrideEyesViewPointForSightPerception))
	FName SightSenseSourceLocationName;

	/** Console variable that toggles Gameplay Camera and Camera Component setups. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Player Camera")
	FString GameplayCameraConsoleVariable;

	/**
	 * Default gameplay effect used when melee damage is applied.
	 *
	 * Can be adjusted by curves/calculations/modifiers, or fully overwritten by the melee
	 * ability itself (or any melee scan executed by it).
	 *
	 * You can also provide dynamic values as needed by overwriting "GetHitEffectClass",
	 * from the Combat Melee Interface, implemented by this character.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Melee Combat")
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Melee Combat", meta = (UIMin = 0.f, ClampMin = 0.f))
	float DefaultMeleeEffectLevel;

	/** Registers all stimuli sources when we have a valid world. */
	void RegisterStimuliSources();
	
	/** Setups the character movement for bots and player characters. */
	void SetupCharacterMovement();
	
	/** Configures movement-related features for an AI controlled pawn. */
	virtual void SetupBotMovement();

	/** Configures movement-related features for a Player controlled pawn. */
	virtual void SetupPlayerMovement();
	
	/**
	 * Initializes the camera, either Gameplay Camera or Camera Component.
	 * 
	 * Ideally, this should be called when the character is possessed. This function will rely
	 * on a Player Controller, so being possessed by an AI has basically no effect.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GASP|Character", meta = (ForceAsFunction))
	void SetupPlayerCamera();
	
	/**
	 * Disables the Gameplay Camera or Camera Component.
	 *
	 * Ideally, this should be called when the character is unpossessed. This function will safely
	 * check for current components and de-initialize them correctly, whether they are based on
	 * the Gameplay Camera system or default Camera Component.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GASP|Character", meta = (ForceAsFunction))
	void DisablePlayerCamera();
	
	/**
	 * Invoked when a gameplay camera activates.
	 * May be used to add camera rigs to layers.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GASP|Character", meta = (ForceAsFunction))
	void OnPlayerCameraActivated(APlayerController* PlayerController);
	virtual void OnPlayerCameraActivated_Implementation(APlayerController* PlayerController) { }

	/**
	 * Invoked when a gameplay camera deactivates.
	 * At this point, the player controller already unpossessed the pawn.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GASP|Character", meta = (ForceAsFunction))
	void OnPlayerCameraDeactivated();
	virtual void OnPlayerCameraDeactivated_Implementation() { }
	
private:

	/** Cached value, defined when the character is possessed. */
	bool bIsLocallyControlled;
	
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

	/** Weapon Manager integrated with the Inventory/Equipment system. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNinjaCombatBaseWeaponManagerComponent> WeaponManager;
	
	/**
	 * Manages all items assigned to this character.
	 * This is an optional component, as it may be retrieved from the Player State.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNinjaInventoryManagerComponent> InventoryManager;

	/** Equipment manager that handles physical presence of items in the world. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNinjaEquipmentManagerComponent> EquipmentManager;
	
	UFUNCTION(Client, Reliable)
	void Client_SetupPlayerCamera();

	UFUNCTION(Client, Reliable)
	void Client_DisablePlayerCamera();

};
