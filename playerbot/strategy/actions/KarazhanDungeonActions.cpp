
#include "playerbot/playerbot.h"
#include "KarazhanDungeonActions.h"
#include "playerbot/strategy/Action.h"

using namespace ai;

bool RemoveNetherPortalBuffsFromNetherspiteAction::Execute(Event& event)
{
	Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
	if (target && ai->HasAura(30466, target))
		target->RemoveAurasDueToSpell(30466);

	if (target && ai->HasAura(30467, target))
		target->RemoveAurasDueToSpell(30467);

	if (target && ai->HasAura(30468, target))
		target->RemoveAurasDueToSpell(30468);

	return true;
}

bool NetherspiteInfernalMoveAwayAction::Execute(Event& event)
{
	Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
	if (target && target->GetEntry() == 15690)
	{
		Unit* tot = target->GetVictim();
		if (tot && tot == bot)
		{
			healersSafe = false;
		}
	}

	return MoveAwayFromCreature::Execute(event);
}