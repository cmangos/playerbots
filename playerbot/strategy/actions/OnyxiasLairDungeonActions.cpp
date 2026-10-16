
#include "playerbot/playerbot.h"
#include "OnyxiasLairDungeonActions.h"

using namespace ai;

bool OnyxiaMoveToSideAction::Execute(Event& /*event*/)
{
    ObjectGuid boss_guid = AI_VALUE2(ObjectGuid, "find target", "onyxia");
    if (!boss_guid)
    {
        // bot->Yell("Onyxia not found!", LANG_UNIVERSAL);
        return false;
    }

    Unit* boss = ai->GetUnit(boss_guid);

    float angleToBot = boss->GetAngle(bot);
    float bossFacing = boss->GetOrientation();
    float diff = fabs(angleToBot - bossFacing);
    if (diff > M_PI)
        diff = 2 * M_PI - diff;

    float distance = bot->GetDistance(boss);

    // Too close (30 yards) and either in front or behind
    if (distance <= 30.0f && (diff < M_PI / 4 || diff > 3 * M_PI / 4))
    {
        float offsetAngle = bossFacing + M_PI / 2; // 90° to the right
        float offsetDist = 15.0f;

        float sideX = boss->GetPositionX() + offsetDist * cos(offsetAngle);
        float sideY = boss->GetPositionY() + offsetDist * sin(offsetAngle);

        // bot->Yell("Too close to front or tail — moving to side of Onyxia!", LANG_UNIVERSAL);
        return MoveTo(boss->GetMapId(), sideX, sideY, boss->GetPositionZ(), false, false, false, false);
    }

    return false;
}

bool OnyxiaSpreadOutAction::Execute(Event& /*event*/)
{
    ObjectGuid boss_guid = AI_VALUE2(ObjectGuid, "find target", "onyxia");
    if (!boss_guid)
        return false;

    Unit* boss = ai->GetUnit(boss_guid);

    // Trigger may fire on one tick, but the action can execute on a later tick.
    // By that time the cast may have finished, so current spell can be null.
    Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell || !currentSpell->m_spellInfo)
        return false;

    // Fireball
    if (currentSpell->m_spellInfo->Id != 18392)
        return false;

    Unit* unitTarget = currentSpell->m_targets.getUnitTarget();
    if (!unitTarget || unitTarget->GetObjectGuid() != bot->GetObjectGuid())
        return false;

    // bot->Yell("Spreading out — I'm the Fireball target!", LANG_UNIVERSAL);
    return MoveFromGroup(9.0f); // move 9 yards
}

bool OnyxiaMoveToSafeZoneAction::Execute(Event& /*event*/)
{
    ObjectGuid boss_guid = AI_VALUE2(ObjectGuid, "find target", "onyxia");
    if (!boss_guid)
        return false;

    Unit* boss = ai->GetUnit(boss_guid);

    Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell || !currentSpell->m_spellInfo)
        return false;

    uint32 spellId = currentSpell->m_spellInfo->Id;

    std::vector<SafeZone> safeZones = GetSafeZonesForBreath(spellId);
    if (safeZones.empty())
        return false;

    // Find closest safe zone
    SafeZone* bestZone = nullptr;
    float bestDist = std::numeric_limits<float>::max();

    for (auto& zone : safeZones)
    {
        float dist = bot->GetDistance2d(zone.pos.GetPositionX(), zone.pos.GetPositionY());
        if (dist < bestDist)
        {
            bestDist = dist;
            bestZone = &zone;
        }
    }

    if (!bestZone)
        return false;

    if (bot->IsWithinDist2d(bestZone->pos.GetPositionX(), bestZone->pos.GetPositionY(), bestZone->radius))
        return false; // Already safe

    // Stop channeling spell first
    bot->AttackStop();
    bot->CastStop();

    // bot->Yell("Moving to Safe Zone!", LANG_UNIVERSAL);
    return MoveTo(bot->GetMapId(), bestZone->pos.GetPositionX(), bestZone->pos.GetPositionY(), bestZone->pos.GetPositionZ(), false, false, false, false);
}

bool OnyxiaKillWhelpsAction::Execute(Event& /*event*/)
{
    ObjectGuid currentTarget_guid = AI_VALUE(ObjectGuid, "current target");
    if (!currentTarget_guid)
        return false;

    Unit* currentTarget = ai->GetUnit(currentTarget_guid);

    // If already attacking a whelp, don't swap targets
    if (currentTarget && currentTarget->GetEntry() == 11262)
    {
        return false;
    }
    std::list<ObjectGuid> targets = AI_VALUE(std::list<ObjectGuid>, "possible targets");
    for (ObjectGuid guid : targets)
    {
        Creature* unit = ai->GetCreature(guid);
        if (!unit || !unit->IsAlive() || !unit->IsInWorld())
            continue;

        if (unit->GetEntry() == 11262) // Onyxia Whelp
        {
            // bot->Yell("Attacking Whelps!", LANG_UNIVERSAL);
            return bot->Attack(unit, !ai->IsRanged(bot));
        }
    }
    return false;
}

bool OnyxiaAvoidEggsAction::Execute(Event& /*event*/)
{
    Position botPos = Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

    float x, y;

    // get safe zone slightly away from eggs (Can this be dynamic?)
    if (botPos.GetDistance2d(Position(- 36.0f, -164.0f, 0.0f)) <= 5.0f)
    {
        x = -10.0f;
        y = -180.0f;
    }
    else if (botPos.GetDistance2d(Position(-34.0f, -262.0f, 0.0f)) <= 5.0f)
    {
        x = -16.0f;
        y = -250.0f;
    }
    else
    {
        return false; // Not in danger zone
    }

    // bot->Yell("Too close to eggs — backing off!", LANG_UNIVERSAL);

    return MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ(), false, false, false, false);
}
