// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/FCharacterOverlayActivationAnimation.h"
#include "NinjaGASPBaseOverlayDataAsset.generated.h"

#define GASP_BASE_OVERLAY_DATA_ASSET_NAME		TEXT("BaseAnimationOverlayData")

class UAnimInstance;

/**
 * Includes overlay data for the base slot.
 */
UCLASS()
class NINJAGASP_API UNinjaGASPBaseOverlayDataAsset : public UPrimaryDataAsset
{
	
	GENERATED_BODY()

public:

	static const FPrimaryAssetType DataAssetType;

	UNinjaGASPBaseOverlayDataAsset();

	/** Animation instance provided by this overlay data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Overlay Data")
	TSubclassOf<UAnimInstance> OverlayAnimationClass;

	/** Optional information for an activation montage played when the overlay activates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Overlay Data")
	FCharacterOverlayActivationAnimation ActivationMontage; 
	
	// -- Begin Primary Data Asset implementation
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	// -- End Primary Data Asset implementation	
	
};
