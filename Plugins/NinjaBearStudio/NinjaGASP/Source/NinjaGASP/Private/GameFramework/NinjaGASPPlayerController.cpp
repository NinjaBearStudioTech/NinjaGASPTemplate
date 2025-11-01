// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPPlayerController.h"

#include "Components/NinjaInputManagerComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/NinjaGASPCharacter.h"

FName ANinjaGASPPlayerController::InputManagerName = TEXT("InputManager");

ANinjaGASPPlayerController::ANinjaGASPPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bAllowDynamicCharacterSwap = true;
	PlayerTeamId = FGenericTeamId::NoTeam.GetId();
	InputManager = CreateDefaultSubobject<UNinjaInputManagerComponent>(InputManagerName);
}

void ANinjaGASPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	CacheTeamIdFromPlayer();
}

void ANinjaGASPPlayerController::OnUnPossess()
{
	PlayerTeamId = FGenericTeamId::NoTeam.GetId();
	Super::OnUnPossess();
}

void ANinjaGASPPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	CacheTeamIdFromPlayer();
}

FGenericTeamId ANinjaGASPPlayerController::GetGenericTeamId() const
{
	return FGenericTeamId(PlayerTeamId);
}

void ANinjaGASPPlayerController::SpawnCharacter(const TSubclassOf<APawn>& NewPawnClass)
{
	if (!bAllowDynamicCharacterSwap || NewPawnClass == nullptr)
	{
		return;
	}

	Server_SpawnCharacter(NewPawnClass);
}

void ANinjaGASPPlayerController::CacheTeamIdFromPlayer()
{
	const APawn* CurrentPawn = GetPawn();
	if (!IsValid(CurrentPawn))
	{
		return;
	}
	
	const IGenericTeamAgentInterface* PawnAgentInterface = Cast<IGenericTeamAgentInterface>(CurrentPawn);
	if (PawnAgentInterface)
	{
		PlayerTeamId = PawnAgentInterface->GetGenericTeamId().GetId();
	}
}

void ANinjaGASPPlayerController::Server_SpawnCharacter_Implementation(const TSubclassOf<APawn> NewPawnClass)
{
	UWorld* MyWorld = GetWorld();
	if (!IsValid(MyWorld))
	{
		return;
	}
	
	checkf(IsValid(NewPawnClass), TEXT("Sanity Check: A character class was not provided."));
	const FRotator ControlRotationBackup = GetControlRotation();
	
	bool bHasValidTransform = false;
	FTransform SpawnTransform = FTransform::Identity;

	if (const APawn* MyPawn = GetPawn())
	{
		bHasValidTransform = true;
		SpawnTransform = MyPawn->GetTransform();
	}
	else if (AGameModeBase* GameMode = MyWorld->GetAuthGameMode())
	{
		const AActor* PlayerStart = GameMode->FindPlayerStart(this);
		if (IsValid(PlayerStart))
		{
			bHasValidTransform = true;
			SpawnTransform = PlayerStart->GetActorTransform();
		}
	}

	if (bHasValidTransform)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		ANinjaGASPCharacter* NewPawn = MyWorld->SpawnActor<ANinjaGASPCharacter>(NewPawnClass, SpawnTransform, SpawnParameters);
		check(IsValid(NewPawn));

		APawn* CurrentPawn = GetPawn();
		if (IsValid(CurrentPawn))
		{
			CurrentPawn->DetachFromControllerPendingDestroy();
		}

		Possess(NewPawn);
		SetControlRotation(ControlRotationBackup);

		if (IsValid(CurrentPawn))
		{
			CurrentPawn->Destroy();	
		}
	}
}

bool ANinjaGASPPlayerController::Server_SpawnCharacter_Validate(const TSubclassOf<APawn> NewPawnClass)
{
	return bAllowDynamicCharacterSwap;
}
