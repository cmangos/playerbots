
#include "playerbot/playerbot.h"
#include "DKActions.h"


using namespace ai;

bool CastRaiseDeadAction::isPossible()
{
	if (!CastBuffSpellAction::isPossible())
		return false;
	if (AI_VALUE2(uint32, "item count", "corpse dust") > 0)
		return true;
		
	for (auto guid : AI_VALUE(std::list<ObjectGuid>, "nearest corpses"))
	{
		Creature* creature = ai->GetCreature(guid);

		if (!creature)
			continue;

		if (creature->GetCreatureType() != CREATURE_TYPE_HUMANOID)
			continue;

		if (!bot->isHonorOrXPTarget(creature))
			continue;

		return true;
	}

	for (auto guid : AI_VALUE(std::list<ObjectGuid>, "nearest friendly players"))
	{
		Player* player = dynamic_cast<Player*>(ai->GetUnit(guid));

		if (!player)
			continue;

		if (!player->IsDead())
			continue;

		return true;
	}

	for (auto guid : AI_VALUE(std::list<ObjectGuid>, "enemy player targets"))
	{
		Player* player = dynamic_cast<Player*>(ai->GetUnit(guid));

		if (!player)
			continue;

		if (!player->IsDead())
			continue;

		return true;
	}

	return false;
}