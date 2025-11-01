// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NinjaGASPlayerState.h"
#include "Interfaces/InventorySystemInterface.h"
#include "NinjaGASPPlayerState.generated.h"

UCLASS()
class NINJAGASP_API ANinjaGASPPlayerState : public ANinjaGASPlayerState, public IInventorySystemInterface
{
	
	GENERATED_BODY()

public:

	ANinjaGASPPlayerState();

protected:

	static FName InventoryManagerName;
	
private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNinjaInventoryManagerComponent> InventoryManager;
	
};
