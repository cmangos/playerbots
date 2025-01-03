
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
    return ai->HasAura("regrowth", target) || ai->HasAura("rejuvenation", target) || ai->HasAura("life bloom", target) || ai->HasAura("wild growth", target);
}

Value<Unit*>* LifebloomTankTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("party tank without lifebloom", "lifebloom");
}

bool CyclonePvpTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsPlayer() || target->GetDiminishing(DIMINISHING_CYCLONE) >= DIMINISHING_LEVEL_IMMUNE)
        return false;

    // Check bot's health for defensive use
    const uint8 health = AI_VALUE2(uint8, "health", "self target");
    if (health <= sPlayerbotAIConfig.lowHealth && target->GetSelectionGuid() == bot->GetObjectGuid())
    {
        // Defensive CC: Check distance for Cyclone
        if (target->GetDistance(bot) <= 20.0f)
        {
            return true;
        }
    }

    // Avoid targets already crowd-controlled
    if (!PossibleAttackTargetsValue::HasUnBreakableCC(target, bot) &&
        target->GetDistance(bot) <= 20.0f)
    {
        // Check if Cyclone is already active
        std::list<ObjectGuid> attackers = AI_VALUE(std::list<ObjectGuid>, "attackers");

        if (attackers.size() <= 1)
            return false;

        for (const auto& guid : attackers)
        {
            Unit* attacker = ai->GetUnit(guid);
            if (ai->HasAura("cyclone", attacker, false, true))
                return false; // Cyclone already active on another target
        }
        return true;
    }

    return false;
}
