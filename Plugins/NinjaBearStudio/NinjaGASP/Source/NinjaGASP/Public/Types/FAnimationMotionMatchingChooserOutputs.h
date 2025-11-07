// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FAnimationMotionMatchingChooserOutputs.generated.h"

class UAnimationAsset;
class UBlendProfile;

/**
 * Stores the output from the State Machine chooser table.
 */
USTRUCT(BlueprintType)
struct NINJAGASP_API FAnimationMotionMatchingChooserOutputs
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation MM Chooser Outputs")
	bool bUseMotionMatching = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation MM Chooser Outputs")
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation MM Chooser Outputs")
	float MotionMatchingCostLimit = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation MM Chooser Outputs")
	float BlendTime = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation MM Chooser Outputs")
	FName BlendProfileName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation MM Chooser Outputs")
	TArray<FName> Tags;
	
};
