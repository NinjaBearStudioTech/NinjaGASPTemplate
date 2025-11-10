// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/FCharacterOverlayActivationAnimation.h"
#include "NinjaGASPPoseOverlayDataAsset.generated.h"

#define GASP_POSE_OVERLAY_DATA_ASSET_NAME		TEXT("PoseAnimationOverlayData")

class UAnimInstance;

UENUM(BlueprintType)
enum class EPoseOverlayItemType : uint8
{
	None,
	StaticMesh,
	SkeletalMesh
};

/**
 * Includes overlay data for the pose slot.
 *
 * Please keep in mind that, even though this is the "GASP-ALS" way to add overlays with items,
 * in this template you would usually add item-related overlays using inventory/equipment data. 
 */
UCLASS()
class NINJAGASP_API UNinjaGASPPoseOverlayDataAsset : public UPrimaryDataAsset
{
	
	GENERATED_BODY()

public:

	static const FPrimaryAssetType DataAssetType;

	UNinjaGASPPoseOverlayDataAsset();

	/** Animation instance provided by this overlay data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pose Overlay Data")
	TSubclassOf<UAnimInstance> OverlayAnimationClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pose Overlay Data")
	EPoseOverlayItemType OverlayItemType = EPoseOverlayItemType::None;

	/** Static Mesh related to this overlay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pose Overlay Data", meta = (EditCondition = "OverlayItemType == EPoseOverlayItemType::StaticMesh"))
	TObjectPtr<UStaticMesh> StaticMesh;

	/** Skeletal Mesh related to this overlay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pose Overlay Data", meta = (EditCondition = "OverlayItemType == EPoseOverlayItemType::SkeletalMesh"))
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	/** Optional Animation class related to the mesh. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pose Overlay Data", meta = (EditCondition = "OverlayItemType == EPoseOverlayItemType::SkeletalMesh"))
	TSubclassOf<UAnimInstance> SkeletalMeshAnimClass;

	/** Informs if the attached item should be oriented as left-handed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pose Overlay Data", meta = (EditCondition = "OverlayItemType != EPoseOverlayItemType::None"))
	bool bIsLeftHand = false;

	/** Socket used to attach the item to the character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pose Overlay Data", meta = (EditCondition = "OverlayItemType != EPoseOverlayItemType::None"))
	FName SocketName = NAME_None;
	
	/** Optional information for an activation montage played when the overlay activates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pose Overlay Data")
	FCharacterOverlayActivationAnimation ActivationMontage; 
	
	// -- Begin Primary Data Asset implementation
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	// -- End Primary Data Asset implementation	
	
};
