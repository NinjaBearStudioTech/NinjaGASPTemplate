// Ninja Bear Studio Inc., all rights reserved.
#include "Data/NinjaGASPBaseOverlayDataAsset.h"

const FPrimaryAssetType	UNinjaGASPBaseOverlayDataAsset::DataAssetType = GASP_BASE_OVERLAY_DATA_ASSET_NAME;

UNinjaGASPBaseOverlayDataAsset::UNinjaGASPBaseOverlayDataAsset()
{
}

FPrimaryAssetId UNinjaGASPBaseOverlayDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(DataAssetType, GetFName());
}
