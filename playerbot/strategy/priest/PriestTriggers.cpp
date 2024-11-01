
#include "playerbot/playerbot.h"
#include "PriestTriggers.h"
#include "PriestActions.h"

using namespace ai;

bool ShadowformTrigger::IsActive()
{
    return !ai->HasAura("shadowform", bot);
}

bool ShadowfiendTrigger::IsActive()
{
    return BoostTrigger::IsActive() && bot->IsSpellReady(34433);
}

bool ManaBurnTrigger::IsActive()
{
	Unit* target = GetTarget();
	if (!target)
		return false;

	bool isHealer = false;
	bool hasMana = false;
	if (target->IsPlayer())
	{
		isHealer = !ai->IsHeal((Player*)target);
		hasMana = AI_VALUE2(uint8, "mana", GetTargetName()) >= 10;
	}

	return SpellCanBeCastedTrigger::IsActive() && isHealer && hasMana;
}
