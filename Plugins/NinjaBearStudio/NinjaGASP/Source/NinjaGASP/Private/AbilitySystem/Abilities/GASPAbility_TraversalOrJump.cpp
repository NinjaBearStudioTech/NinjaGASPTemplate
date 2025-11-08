// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/GASPAbility_TraversalOrJump.h"

#include "NinjaGASPTags.h"

UGASPAbility_TraversalOrJump::UGASPAbility_TraversalOrJump()
{
	TraceType = TraceTypeQuery1;

	FGameplayTagContainer Tags;
	Tags.AddTagFast(Tag_GASP_Ability_TraversalOrJump);
	SetAssetTags(Tags);
}
