// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Types/EPlayerCameraMode.h"
#include "UObject/Interface.h"
#include "PlayerCameraModeInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPlayerCameraModeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contains information and functionality related to the player camera.
 */
class NINJAGASP_API IPlayerCameraModeInterface
{
	
	GENERATED_BODY()

public:

	/**
	 * Provides the current aiming state for the camera.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Player Camera Mode Interface")
	bool InCameraAiming() const;
	virtual bool InCameraAiming_Implementation() const { return false; }

	/**
	 * Sets a new aiming state for the camera. 
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Player Camera Mode Interface")
	void SetCameraAiming(bool bNewCameraAiming);
	virtual void SetCameraAiming_Implementation(bool bNewCameraAiming) { }
	
	/**
	 * Provides the current camera mode.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "NBS|GASP|Player Camera Mode Interface")
	EPlayerCameraMode GetCameraMode() const;
	virtual EPlayerCameraMode GetCameraMode_Implementation() const { return EPlayerCameraMode::Balanced; }
	
};
