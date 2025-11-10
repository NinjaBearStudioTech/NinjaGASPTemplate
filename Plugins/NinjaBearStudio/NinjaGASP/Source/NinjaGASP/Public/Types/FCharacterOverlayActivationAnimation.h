// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FCharacterOverlayActivationAnimation.generated.h"

class UAnimSequenceBase;

/**
 * Animation Sequence (Montage, Composite) that will dynamically play when an overlay activates.
 */
USTRUCT(BlueprintType)
struct NINJAGASP_API FCharacterOverlayActivationAnimation
{
	
	GENERATED_BODY()

	/** Montage played when an overlay activates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation Animation")
	TObjectPtr<UAnimSequenceBase> DynamicAnimation = nullptr;

	/** Slot used to play the animation montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation Animation")
	FName SlotName = TEXT("Transition");

	/** Blend in time when the montage plays. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation Animation")
	float BlendInTime = 0.1f;

	/** Blend out time when the montage plays. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation Animation")
	float BlendOutTime = 0.1f;

	/** Play rate for the animation montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation Animation")
	float PlayRate = 1.0f;

	/** Time when the montage starts playing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation Animation")
	float StartTime = 0.f;
	
	/** Blend out trigger time for the montage playing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation Animation")
	float BlendOutTriggerTime = -1.f;
	
	/** Loop count for the montage to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation Animation")
	int32 LoopCount = 1;

};