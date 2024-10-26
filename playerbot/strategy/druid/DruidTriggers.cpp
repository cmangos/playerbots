
#include "playerbot/playerbot.h"
#include "DruidTriggers.h"
#include "DruidActions.h"

using namespace ai;

bool EntanglingRootsKiteTrigger::IsActive() 
{ 
	if (!DebuffTrigger::IsActive())
		return false;

    if (AI_VALUE(uint8, "attackers count") > 3)
        return false;

	if (!GetTarget()->HasMana())
		return false;

    std::list<ObjectGuid> attackers = context->GetValue<std::list<ObjectGuid>>("attackers")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit || !sServerFacade.IsAlive(unit))
            continue;

        if (ai->HasMyAura("entangling roots", unit))
            return false;
    }

    return !HasMaxDebuffs();
}

bool ActiveHotTrigger::IsActive()
{
    Unit* target = GetTarget(); 
    uint32 regrowth = AI_VALUE2(uint32, "spell id", "regrowth");
    uint32 rejuvenation = AI_VALUE2(uint32, "spell id", "rejuvenation");
    if (target && !(regrowth || rejuvenation))
        return false;
    return ai->HasAura("regrowth", target) || ai->HasAura("rejuvenation", target);
}

Value<Unit*>* LifebloomTankTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("party tank without lifebloom", "lifebloom");
}