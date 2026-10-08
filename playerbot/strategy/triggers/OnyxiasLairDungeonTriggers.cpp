
#include "playerbot/playerbot.h"
#include "playerbot/strategy/Trigger.h"
#include "OnyxiasLairDungeonTriggers.h"

using namespace ai;

OnyxiaDeepBreathTrigger::OnyxiaDeepBreathTrigger(PlayerbotAI* ai) : Trigger(ai, "ony deep breath warning") {}

bool OnyxiaDeepBreathTrigger::IsActive()
{
    ObjectGuid boss_guid = AI_VALUE2(ObjectGuid, "find target", "onyxia");
    if (!boss_guid)
        return false;

    Unit* boss = ai->GetUnit(boss_guid);

    // Check if Onyxia is casting
    Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);

    if (!currentSpell || !currentSpell->m_spellInfo)
        return false;

    uint32 spellId = currentSpell->m_spellInfo->Id;

    if (spellId == 17086 || // North to South
        spellId == 18351 || // South to North
        spellId == 18576 || // East to West
        spellId == 18609 || // West to East
        spellId == 18564 || // Southeast to Northwest
        spellId == 18584 || // Northwest to Southeast
        spellId == 18596 || // Southwest to Northeast
        spellId == 18617    // Northeast to Southwest
    )
    {
        return true;
    }

    return false;
}

OnyxiaNearTailTrigger::OnyxiaNearTailTrigger(PlayerbotAI* ai) : Trigger(ai, "ony near tail") {}

bool OnyxiaNearTailTrigger::IsActive()
{
    ObjectGuid boss_guid = AI_VALUE2(ObjectGuid, "find target", "onyxia");
    if (!boss_guid)
    {
        // bot->Yell("Onyxia not found!", LANG_UNIVERSAL);
        return false;
    }

    Unit* boss = ai->GetUnit(boss_guid);
    if (!boss || ai->IsTank(bot))
        return false;

    // Skip if Onyxia is in air or transitioning
    if (!boss->IsInCombat() || boss->IsFlying() || !boss->GetVictim())
        return false;

    return true;
}

OnyxiaFireballSplashTrigger::OnyxiaFireballSplashTrigger(PlayerbotAI* ai)
    : Trigger(ai, "ony fireball splash incoming")
{
}

bool OnyxiaFireballSplashTrigger::IsActive()
{
    ObjectGuid boss_guid = AI_VALUE2(ObjectGuid, "find target", "onyxia");
    if (!boss_guid)
        return false;

    Unit* boss = ai->GetUnit(boss_guid);
    if (!boss || !boss->IsNonMeleeSpellCasted(false))
        return false;

    // Check if Onyxia is casting Fireball
    Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell || !currentSpell->m_spellInfo || currentSpell->m_spellInfo->Id != 18392) // 18392 is the classic Fireball ID
        return false;

    std::list<ObjectGuid> nearbyUnits = AI_VALUE(std::list<ObjectGuid>, "nearest friendly players");
    for (ObjectGuid guid : nearbyUnits)
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || unit == bot || !unit->IsAlive())
            continue;

        if (bot->GetDistance(unit) < 8.0f)
            return true;
    }

    return false;
}

OnyxiaWhelpsSpawnTrigger::OnyxiaWhelpsSpawnTrigger(PlayerbotAI* ai) : Trigger(ai, "ony whelps spawn") {}

bool OnyxiaWhelpsSpawnTrigger::IsActive()
{
    ObjectGuid boss_guid = AI_VALUE2(ObjectGuid, "find target", "onyxia");
    if (!boss_guid)
        return false;

    Unit* boss = ai->GetUnit(boss_guid);

    return !ai->IsHeal(bot) && boss->IsFlying(); // DPS + Tanks only
}

OnyxiaAvoidEggsTrigger::OnyxiaAvoidEggsTrigger(PlayerbotAI* ai) : Trigger(ai, "ony avoid eggs") {}

bool OnyxiaAvoidEggsTrigger::IsActive()
{
    Position botPos = Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

    if (botPos.GetDistance2d(Position(-35.0f, -165.0f, 0.0f)) <= 5.0f)
        return true;

    if (botPos.GetDistance2d(Position(-35.0f, -260.0f, 0.0f)) <= 5.0f)
        return true;

    return false;
}
