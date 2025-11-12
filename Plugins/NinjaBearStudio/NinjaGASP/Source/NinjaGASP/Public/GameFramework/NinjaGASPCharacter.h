// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NinjaGASCharacter.h"
#include "Interfaces/AdvancedCharacterMovementInterface.h"
#include "Interfaces/CombatMeleeInterface.h"
#include "Interfaces/CombatRangedInterface.h"
#include "Interfaces/CombatSystemInterface.h"
#include "Interfaces/EquipmentSystemInterface.h"
#include "Interfaces/InventoryInitializationWatcherInterface.h"
#include "Interfaces/InventorySystemInterface.h"
#include "Interfaces/PreMovementComponentTickInterface.h"
#include "Interfaces/TraversalMovementInputInterface.h"
#include "Types/ECharacterOverlayBase.h"
#include "Types/ECharacterOverlayPose.h"
#include "Types/ECharacterTraversalAction.h"
#include "Types/FCharacterMovementIntents.h"
#include "Types/FCharacterOverlayActivationAnimation.h"
#include "Types/FCharacterTraversalActionSummary.h"
#include "Types/FInventoryDefaultItem.h"
#include "NinjaGASPCharacter.generated.h"

class UAISense;
class UChooserTable;
class USphereComponent;
class UGameplayCameraComponent;
class UNinjaCombatMotionWarpingComponent;
class UAIPerceptionStimuliSourceComponent;
class UNinjaCombatManagerComponent;
class UNinjaCombatComboManagerComponent;
class UNinjaCombatBaseWeaponManagerComponent;
class UNinjaInventoryManagerComponent;
class UNinjaEquipmentManagerComponent;
class UNinjaInteractionManagerComponent;
class UNinjaGASPPoseOverlayDataAsset;

/**
 * Base character, with functionality that can be shared between players and AI.
 */
UCLASS()
class NINJAGASP_API ANinjaGASPCharacter : public ANinjaGASCharacter, public IPreMovementComponentTickInterface,
	public IAdvancedCharacterMovementInterface, public ITraversalMovementInputInterface, public ICombatSystemInterface, 
	public ICombatMeleeInterface, public ICombatRangedInterface, public IInventorySystemInterface, public IEquipmentSystemInterface,
	public IInventoryInitializationWatcherInterface
{

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBaseAnimationOverlayChanged, ECharacterOverlayBase, NewBase);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPoseAnimationOverlayChanged, ECharacterOverlayPose, NewPose);
	
	GENERATED_BODY()

public:

	/** Broadcasts a change in the base animation overlay, providing the new value. */
	UPROPERTY(BlueprintAssignable)
	FBaseAnimationOverlayChanged OnBaseAnimationOverlayChanged;

	/** Broadcasts a change in the pose animation overlay, providing the new value. */
	UPROPERTY(BlueprintAssignable)
	FPoseAnimationOverlayChanged OnPoseAnimationOverlayChanged;
	
	ANinjaGASPCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Pawn/Character implementation
	virtual bool IsLocallyControlled() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
	virtual void OnRep_Controller() override;
	virtual void OnJumped_Implementation() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;
	// -- End Pawn/Character implementation

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
	// -- End Combat/Melee/Ranged implementation
	
	// -- Begin Inventory/Equipment implementation
	virtual UNinjaInventoryManagerComponent* GetInventoryManager_Implementation() const override;
	virtual UNinjaEquipmentManagerComponent* GetEquipmentManager_Implementation() const override;
	virtual void OnInventoryInitializationFinished_Implementation(UNinjaInventoryManagerComponent* Inventory) override;
	// -- End Inventory/Equipment implementation

	// -- Begin Advanced Movement implementation
	virtual void SetWalkingIntent_Implementation(bool bWantsToWalk) override;
	virtual void SetSprintingIntent_Implementation(bool bWantsToSprint) override;
	virtual void SetStrafingIntent_Implementation(bool bWantsToStrafe) override;
	virtual void SetAimingIntent_Implementation(bool bWantsToAim) override;
	virtual bool IsActivelyRunning_Implementation() const override;
	virtual bool IsActivelySprinting_Implementation() const override;
	virtual bool HasJustLanded_Implementation() const override { return bJustLanded; }
	virtual FVector GetLandVelocity_Implementation() const override { return LandedVelocity; }
	// -- End Advanced Movement implementation

	/**
	 * Provides a copy of the current movement intents.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Character")	
	FCharacterMovementIntents GetMovementIntents() const;
	
	/**
	 * Updates the internal movement intents using the provided values.
	 *
	 * This will set the movement in the local player and server. If the local player is
	 * not authoritative, then this function will also call the server via an RPC.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Character")
	void SetMovementIntents(FCharacterMovementIntents NewMovementIntents);
	
	/**
	 * Checks if the character should use Gameplay Cameras, based on the cvar.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Character")
	bool ShouldUseGameplayCameras() const;

	/**
	 * Informs if there is an active traversal action currently registered.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Character")
	FCharacterTraversalActionSummary GetActiveTraversalAction() const;
	
	/**
	 * Registers a traversal action with a given target.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Character")
	void RegisterTraversalAction(ECharacterTraversalAction ActionType, UPrimitiveComponent* Target);

	/**
	 * Notifies the end of any ongoing traversal action.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Character")
	void ClearTraversalAction();

	/**
	 * Returns the overlay value that affects the base animation layer.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Character")
	ECharacterOverlayBase GetBaseAnimationOverlay() const { return BaseAnimationOverlay; }

	/**
	 * Sets the overlay value that affects the base animation layer.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Character")
	void SetBaseAnimationOverlay(const ECharacterOverlayBase NewBase);

	/**
	 * Returns the overlay value that affects the pose animation layer.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GASP|Character")
	ECharacterOverlayPose GetPoseAnimationOverlay() const { return PoseAnimationOverlay; }

	/**
	 * Sets the overlay value that affects the pose animation layer.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GASP|Character")
	void SetPoseAnimationOverlay(const ECharacterOverlayPose NewPose);

	/**
	 * Attaches an object related to a pose overlay.
	 *
	 * Usually, this is mostly related to the default GASPALS object attachment and this function
	 * is simply exposing that. But as you get more and more into the Inventory system integration,
	 * this should become "prototype code".
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character", meta = (ForceAsFunction))
	void AttachPoseObjectToHand(const UNinjaGASPPoseOverlayDataAsset* PoseData);
	virtual void AttachPoseObjectToHand_Implementation(const UNinjaGASPPoseOverlayDataAsset* PoseData);
	
	/**
	 * Clears any object directly held by the character.
	 *
	 * Usually, this is mostly related to the default GASPALS object attachment and this function
	 * is simply exposing that. But as you get more and more into the Inventory system integration,
	 * this should become "prototype code".
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character", meta = (ForceAsFunction))
	void ClearHeldObject();
	virtual void ClearHeldObject_Implementation();
	
protected:

	static FName CombatManagerName;
	static FName ComboManagerName;
	static FName MotionWarpingName;
	static FName WeaponManagerName;
	static FName InventoryManagerName;
	static FName EquipmentManagerName;
	static FName InteractionManagerName;
	static FName InteractionScanName;

	/** Absolute yaw we must be in to be considered actively sprinting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Movement", meta = (UIMin = 0.f, ClampMin = 0.f))
	float ActiveSprintAngle;
	
	/** Time to wait, before resetting the landed flag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Movement", meta = (UIMin = 0.f, ClampMin = 0.f))
	float LandingResetTime;

	/** Interval to reset corrections after a traversal movement ends. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Movement", meta = (UIMin = 0.f, ClampMin = 0.f))
	float TimeToResetTraversalCorrections;
	
	/** All stimuli sources registered with this character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|AI")
	TArray<TSubclassOf<UAISense>> StimuliSenseSources;

	/** If enabled, allows overriding the source for Sight perception to this bone or socket. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|AI", meta = (InlineEditConditionToggle))
	bool bOverrideEyesViewPointForSightPerception;

	/** Bone or socket used as the source (location and rotation) used by the Sight sense as "eye location". */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|AI", meta = (EditCondition = bOverrideEyesViewPointForSightPerception))
	FName SightSenseSourceLocationName;

	/** The base overlay used for the animation. During gameplay, use the provided getter and setter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Overlay", ReplicatedUsing = OnRep_BaseAnimationOverlay)
	ECharacterOverlayBase BaseAnimationOverlay;
	
	/** The pose overlay used for the animation. During gameplay, use the provided getter and setter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Overlay", ReplicatedUsing = OnRep_PoseAnimationOverlay)
	ECharacterOverlayPose PoseAnimationOverlay;

	/** Chooser table used for base overlays. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Overlay")
	TObjectPtr<UChooserTable> BaseOverlayChooserTable;

	/** Chooser table used for pose overlays. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Overlay")
	TObjectPtr<UChooserTable> PoseOverlayChooserTable;

	/** Default items added to the inventory when it initializes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Inventory", meta = (TitleProperty = "ItemData"))
	TArray<FInventoryDefaultItem> InitialItems;

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

	/** Console variable that toggles Gameplay Camera and Camera Component setups. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASP|Player Camera")
	FString GameplayCameraConsoleVariable;
	
	/** Registers all stimuli sources when we have a valid world. */
	void RegisterStimuliSources();

	/** Consolidates the base animation layer, processing the current value. */
	void ConsolidateBaseAnimationLayer();

	/** Consolidates the pose animation layer, processing the current value. */
	void ConsolidatePoseAnimationLayer();

	/** Links the overlay instance to the correct animation, provided by combat. */
	void LinkOverlayAnimationLayer(TSubclassOf<UAnimInstance> LayerClass) const;
	
	/** Plays a dynamic montage from the provided montage data. */
	void PlaySlotAnimationAsDynamicMontage(const FCharacterOverlayActivationAnimation& Params) const;

	/**
	 * Timer friendly function that will wait and set the owner to an anim bp.
	 * 
	 * This is usually something the Equipment Instance would do, but the ALS layer system handles
	 * its own overlay items directly, so we also need to account for that here.
	 */
	UFUNCTION()
	void SetOwnerToPoseOverlayAnimationInstance(USkeletalMeshComponent* PoseItemMesh);
	
	/**
	 * Initializes the camera, either Gameplay Camera or Camera Component.
	 * 
	 * Ideally, this should be called when the character is possessed. This function will rely
	 * on a Player Controller, so being possessed by an AI has basically no effect.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character", meta = (ForceAsFunction))
	void SetupPlayerCamera();
	
	/**
	 * Disables the Gameplay Camera or Camera Component.
	 *
	 * Ideally, this should be called when the character is unpossessed. This function will safely
	 * check for current components and de-initialize them correctly, whether they are based on
	 * the Gameplay Camera system or default Camera Component.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character", meta = (ForceAsFunction))
	void DisablePlayerCamera();
	
	/**
	 * Invoked when a gameplay camera activates.
	 * May be used to add camera rigs to layers.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character", meta = (ForceAsFunction))
	void OnPlayerCameraActivated(APlayerController* PlayerController);
	virtual void OnPlayerCameraActivated_Implementation(APlayerController* PlayerController) { }

	/**
	 * Invoked when a gameplay camera deactivates.
	 * At this point, the player controller already unpossessed the pawn.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character", meta = (ForceAsFunction))
	void OnPlayerCameraDeactivated();
	virtual void OnPlayerCameraDeactivated_Implementation() { }

	/**
	 * Custom event that handles the character jumping.
	 *
	 * @param GroundSpeedBeforeJump
	 *		Speed on ground, before the jump started.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character")
	void HandleCharacterJumped(float GroundSpeedBeforeJump);
	
	/**
	 * Custom event that handles the character landing.
	 * Make sure to call super to keep track of the "just landed" flag!
	 *
	 * @param Velocity
	 *		Velocity the character had while landing.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character")
	void HandleCharacterLanded(FVector Velocity);

	/**
	 * Handles a change for the base animation overlay.
	 *
	 * @param CurrentBase
	 *		Current value for the base.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character", meta = (ForceAsFunction))
	void HandleBaseAnimationOverlayChanged(ECharacterOverlayBase CurrentBase);

	/**
	 * Handles a change for the pose animation overlay.
	 *
	 * @param CurrentPose
	 *		Current value for the pose.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NBS|GASP|Character", meta = (ForceAsFunction))
	void HandlePoseAnimationOverlayChanged(ECharacterOverlayPose CurrentPose);
	
	/**
	 * Reacts to the replication of the movement intents.
	 * Includes the previous version for detailed comparison.
	 */
	UFUNCTION()
	virtual void OnRep_MovementIntents(FCharacterMovementIntents OldMovementIntents);

	/**
	 * Reacts to the replication of the traversal action summary.
	 * Includes the previous version for detailed comparison.
	 */
	UFUNCTION()
	virtual void OnRep_TraversalAction(FCharacterTraversalActionSummary OldTraversalActionSummary);

	/**
	 * Reacts to the replication of the base animation overlay.
	 * This is where we should actually apply the pose.
	 */
	UFUNCTION()
	virtual void OnRep_BaseAnimationOverlay();

	/**
	 * Reacts to the replication of the pose animation overlay.
	 * This is where we should actually apply the pose.
	 */
	UFUNCTION()
	virtual void OnRep_PoseAnimationOverlay();
	
	/**
	 * Routes the input state through the server.
	 * Most likely incoming from "SetMovementIntents".
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetMovementIntents(FCharacterMovementIntents InputStateSettings);

	/**
	 * Routes the traversal action through the server.
	 * Most likely incoming from "RegisterTraversalAction".
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RegisterTraversalAction(ECharacterTraversalAction ActionType, UPrimitiveComponent* Target);
	
	/**
	 * Routes the traversal action through the server.
	 * Most likely incoming from "ClearTraversalAction".
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ClearTraversalAction();

	/**
	 * Routes the base animation layer on the server to support replication. 
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetBaseAnimationOverlay(ECharacterOverlayBase NewBase);

	/**
	 * Routes the pose animation layer on the server to support replication. 
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetPoseAnimationOverlay(ECharacterOverlayPose NewPose);
	
private:

	/** Cached value, defined when the character is possessed. */
	bool bIsLocallyControlled;

	/** Informs if the character has just landed. Should reset after an interval. */
	bool bJustLanded;
	
	/** Stores the velocity when the character lands. */
	FVector LandedVelocity;

	/** Timer handle that resets the landed flag. */
	FTimerHandle LandedResetTimerHandle;

	/** Timer handle that resets the corrections from traversal. */
	FTimerHandle TraversalCorrectionTimerHandle;

	/** Tracks the last base layer processed. Track this to avoid applying twice due to prediction. */
	ECharacterOverlayBase LastProcessedBaseOverlay = ECharacterOverlayBase::None;
	
	/** Tracks the last pose layer processed. Track this to avoid applying twice due to prediction. */
	ECharacterOverlayPose LastProcessedPoseOverlay = ECharacterOverlayPose::None;
	
	/** Aggregation of current input flags. */
	UPROPERTY(ReplicatedUsing = OnRep_MovementIntents)
	FCharacterMovementIntents MovementIntents;

	/** Summary of the current traversal action. */
	UPROPERTY(ReplicatedUsing = OnRep_TraversalAction)
	FCharacterTraversalActionSummary TraversalActionSummary;

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

	/** Interaction Manager used to interact with the world. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNinjaInteractionManagerComponent> InteractionManager;

	/** Sphere used to scan for interactable objects. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<USphereComponent> InteractionScan;
	
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
