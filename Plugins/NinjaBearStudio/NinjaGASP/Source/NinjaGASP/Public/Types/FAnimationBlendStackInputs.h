// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FAnimationBlendStackInputs.generated.h"

class UAnimationAsset;
class UBlendProfile;

/**
 * Struct used in Experimental State Machine to drive Blend Stack inputs.
 */
USTRUCT(BlueprintType)
struct FAnimationBlendStackInputs
{
	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Stack Input")
	TObjectPtr<UAnimationAsset> Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Stack Input")
	bool bLoop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Stack Input")
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Stack Input")
	float BlendTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Stack Input")
	TObjectPtr<UBlendProfile> BlendProfile = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Stack Input")
	TArray<FName> Tags;
	
};
