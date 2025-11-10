// Ninja Bear Studio Inc., all rights reserved.
#include "Data/NinjaGASPPoseOverlayDataAsset.h"

const FPrimaryAssetType	UNinjaGASPPoseOverlayDataAsset::DataAssetType = GASP_POSE_OVERLAY_DATA_ASSET_NAME;

UNinjaGASPPoseOverlayDataAsset::UNinjaGASPPoseOverlayDataAsset()
{
}

FPrimaryAssetId UNinjaGASPPoseOverlayDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(DataAssetType, GetFName());
}
