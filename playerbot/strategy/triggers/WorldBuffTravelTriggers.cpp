#include "playerbot/playerbot.h"
#include "WorldBuffTravelTriggers.h"
#include "playerbot/ServerFacade.h"
#include "Server/DBCStores.h"

using namespace ai;

static bool IsNpcNearby(Player* bot, uint32 npcEntry)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
        return false;

    std::list<ObjectGuid> npcs = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("nearest npcs no los")->Get();
    for (auto& guid : npcs)
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || !unit->IsCreature())
            continue;

        if (unit->GetEntry() == npcEntry &&
            bot->GetDistance(unit) <= INTERACTION_DISTANCE)
            return true;
    }
    return false;
}

static bool IsNearDMNorthDoor(Player* bot)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
        return false;

    std::list<ObjectGuid> gos = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("nearest game objects no los")->Get();
    for (auto& guid : gos)
    {
        GameObject* go = ai->GetGameObject(guid);
        if (!go)
            continue;

        if (go->GetEntry() == GO_DM_NORTH_DOOR &&
            bot->GetDistance(go) <= INTERACTION_DISTANCE)
            return true;
    }
    return false;
}

static bool IsPortalNearby(Player* bot, const char* destKeyword)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
        return false;

    std::list<ObjectGuid> gos = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("nearest game objects no los")->Get();
    for (auto& guid : gos)
    {
        GameObject* go = ai->GetGameObject(guid);
        if (!go)
            continue;

        GameObjectInfo const* goInfo = go->GetGOInfo();
        if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_SPELLCASTER)
            continue;

        uint32 spellId = goInfo->spellcaster.spellId;
        const SpellEntry* const pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_TELEPORT_UNITS &&
            pSpellInfo->Effect[1] != SPELL_EFFECT_TELEPORT_UNITS &&
            pSpellInfo->Effect[2] != SPELL_EFFECT_TELEPORT_UNITS)
            continue;

        std::string name = goInfo->name;
        if (name.find(destKeyword) != std::string::npos)
            return true;
    }
    return false;
}

static bool IsNearSongflower(Player* bot)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
        return false;

    std::list<ObjectGuid> gos = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("nearest game objects no los")->Get();
    for (auto& guid : gos)
    {
        GameObject* go = ai->GetGameObject(guid);
        if (!go || !IsSongflowerEntry(go->GetEntry()))
            continue;

        if (bot->GetDistance(go) <= INTERACTION_DISTANCE)
            return true;
    }
    return false;
}

static bool IsNearBonfire(Player* bot)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
        return false;

    std::list<ObjectGuid> gos = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("nearest game objects no los")->Get();
    for (auto& guid : gos)
    {
        GameObject* go = ai->GetGameObject(guid);
        if (!go)
            continue;

        if (go->GetEntry() == GO_BONFIRE_FERALAS &&
            bot->GetDistance(go) <= INTERACTION_DISTANCE)
            return true;
    }
    return false;
}

static bool AreAllGroupMembersInPortalDestZone(Player* bot)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
        return false;

    uint32 destZone = GetDMPortalDestZone(bot);

    std::vector<Player*> members = ai->GetPlayersInGroup();
    for (Player* member : members)
    {
        if (!member)
            continue;

        uint32 zoneId = member->GetZoneId();
        if (zoneId != destZone)
            return false;
    }
    return true;
}

static uint32 ForEachGroupMember(Player* bot, std::function<void(Player*)> callback)
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    uint32 count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || member == bot)
            continue;

        ++count;
        callback(member);
    }
    return count;
}

static bool DoAllGroupMembersHaveDMTBuffs(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return true;

    uint32 checkedCount = 0;
    uint32 passedCount = 0;

    ForEachGroupMember(bot, [&](Player* member)
        {
            ++checkedCount;
            if (HasAllDMTributeBuffs(member))
                ++passedCount;
        });

    uint32 totalMembers = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        if (ref->getSource() && ref->getSource() != bot)
            ++totalMembers;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (member == bot)
            continue;

        if (!member)
            return false;

        if (!HasAllDMTributeBuffs(member))
            return false;
    }

    return true;
}

static bool DoAllGroupMembersHaveSongflower(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return true;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (member == bot)
            continue;

        if (!member)
            return false;

        if (!member->HasAura(SPELL_SONGFLOWER))
            return false;
    }

    return true;
}

static bool IsInStepZone(Player* bot, WorldBuffTravelStep step)
{
    uint32 zoneId = 0;
    uint32 areaId = 0;
    bot->GetZoneAndAreaId(zoneId, areaId);

    bool horde = IsHordeFaction(bot);

    switch (step)
    {
    case WorldBuffTravelStep::STEP_STORMWIND:
        return zoneId == GetHomeZone(bot);
    case WorldBuffTravelStep::STEP_GRYPHON_MASTER:
        // Alliance only — Horde skips this step entirely
        return horde ? false : IsNpcNearby(bot, NPC_DUNGAR_LONGDRINK);
    case WorldBuffTravelStep::STEP_BOOTY_BAY:
        return areaId == AREA_BOOTY_BAY;
    case WorldBuffTravelStep::STEP_BRAGOK:
        return IsNpcNearby(bot, NPC_BRAGOK);
    case WorldBuffTravelStep::STEP_THYSSIANA:
        return horde ? IsNpcNearby(bot, NPC_SHYN) : IsNpcNearby(bot, NPC_THYSSIANA);
    case WorldBuffTravelStep::STEP_FEATHERMOON:
        // Alliance only — Horde skips this step entirely
        return horde ? false : (zoneId == ZONE_FERALAS);
    case WorldBuffTravelStep::STEP_FORGOTTEN_COAST:
        return horde ? IsNearBonfire(bot) : IsNpcNearby(bot, NPC_ZORBIN_FANDAZZLE);
    case WorldBuffTravelStep::STEP_DM_TRAVEL:
        return IsNearDMNorthDoor(bot);
    case WorldBuffTravelStep::STEP_FELWOOD:
        return horde ? IsNpcNearby(bot, NPC_BRAKKAR) : IsNpcNearby(bot, NPC_MISHELLENA);
    case WorldBuffTravelStep::STEP_SONGFLOWER:
    {
        if (bot->HasAura(SPELL_SONGFLOWER))
            return true;

        return IsNearSongflower(bot);
    }
    default:
        return false;
    }
}

bool WorldBuffTravelZoneReachedTrigger::IsActive()
{
    uint8 step = AI_VALUE(uint8, "world buff travel step");
    WorldBuffTravelStep s = static_cast<WorldBuffTravelStep>(step);

    // Horde skips Alliance-only steps
    if (IsHordeFaction(bot) && IsAllianceOnlyStep(s))
        return false;

    if (s != WorldBuffTravelStep::STEP_STORMWIND &&
        s != WorldBuffTravelStep::STEP_GRYPHON_MASTER &&
        s != WorldBuffTravelStep::STEP_BOOTY_BAY &&
        s != WorldBuffTravelStep::STEP_BRAGOK &&
        s != WorldBuffTravelStep::STEP_THYSSIANA &&
        s != WorldBuffTravelStep::STEP_FEATHERMOON &&
        s != WorldBuffTravelStep::STEP_FORGOTTEN_COAST &&
        s != WorldBuffTravelStep::STEP_DM_TRAVEL &&
        s != WorldBuffTravelStep::STEP_FELWOOD &&
        s != WorldBuffTravelStep::STEP_SONGFLOWER)
        return false;

    return IsInStepZone(bot, s);
}

bool WorldBuffTravelNeedMoveTrigger::IsActive()
{
    if (bot->IsTaxiFlying())
        return false;

    uint8 step = AI_VALUE(uint8, "world buff travel step");
    WorldBuffTravelStep s = static_cast<WorldBuffTravelStep>(step);

    if (IsHordeFaction(bot) && IsAllianceOnlyStep(s))
        return false;

    if (s != WorldBuffTravelStep::STEP_STORMWIND &&
        s != WorldBuffTravelStep::STEP_GRYPHON_MASTER &&
        s != WorldBuffTravelStep::STEP_BOOTY_BAY &&
        s != WorldBuffTravelStep::STEP_BRAGOK &&
        s != WorldBuffTravelStep::STEP_THYSSIANA &&
        s != WorldBuffTravelStep::STEP_FEATHERMOON &&
        s != WorldBuffTravelStep::STEP_FORGOTTEN_COAST &&
        s != WorldBuffTravelStep::STEP_DM_TRAVEL &&
        s != WorldBuffTravelStep::STEP_DM_PORTAL &&
        s != WorldBuffTravelStep::STEP_FELWOOD &&
        s != WorldBuffTravelStep::STEP_SONGFLOWER &&
        s != WorldBuffTravelStep::STEP_PORTAL_HOME)
        return false;

    if (s == WorldBuffTravelStep::STEP_DM_PORTAL)
    {
        uint32 destZone = GetDMPortalDestZone(bot);
        if (bot->GetZoneId() == destZone)
            return false;

        return !IsInStepZone(bot, WorldBuffTravelStep::STEP_DM_TRAVEL);
    }

    if (s == WorldBuffTravelStep::STEP_PORTAL_HOME)
    {
        uint32 homeZone = GetHomeZone(bot);
        if (bot->GetZoneId() == homeZone)
            return false;

        if (bot->IsBeingTeleported())
            return false;

        return !IsNearSongflower(bot);
    }

    return !IsInStepZone(bot, s);
}

bool WorldBuffTravelDMBuffedTrigger::IsActive()
{
    uint8 step = AI_VALUE(uint8, "world buff travel step");
    if (step != static_cast<uint8>(WorldBuffTravelStep::STEP_DM_INSIDE))
        return false;

    return HasAllDMTributeBuffs(bot);
}

bool WorldBuffTravelDMExitedTrigger::IsActive()
{
    uint8 step = AI_VALUE(uint8, "world buff travel step");
    if (step != static_cast<uint8>(WorldBuffTravelStep::STEP_DM_EXIT))
        return false;

    return HasAllDMTributeBuffs(bot);
}

bool WorldBuffTravelDMPortalCastTrigger::IsActive()
{
    uint8 step = AI_VALUE(uint8, "world buff travel step");
    if (step != static_cast<uint8>(WorldBuffTravelStep::STEP_DM_PORTAL))
        return false;

    if (bot->getClass() != CLASS_MAGE)
        return false;

    if (!HasAllDMTributeBuffs(bot))
        return false;

    if (!DoAllGroupMembersHaveDMTBuffs(bot))
        return false;

    if (!AreAllGroupMembersNearby(bot))
        return false;

    if (IsPortalNearby(bot, GetDMPortalKeyword(bot)))
        return false;

    return true;
}

bool WorldBuffTravelDMPortalUseTrigger::IsActive()
{
    uint8 step = AI_VALUE(uint8, "world buff travel step");
    if (step != static_cast<uint8>(WorldBuffTravelStep::STEP_DM_PORTAL))
        return false;

    if (!HasAllDMTributeBuffs(bot))
        return false;

    return IsPortalNearby(bot, GetDMPortalKeyword(bot));
}

bool WorldBuffTravelPortalStepTrigger::IsActive()
{
    uint8 step = AI_VALUE(uint8, "world buff travel step");
    if (step != static_cast<uint8>(WorldBuffTravelStep::STEP_PORTAL_HOME))
        return false;

    if (bot->getClass() != CLASS_MAGE)
        return false;

    if (!DoAllGroupMembersHaveSongflower(bot))
        return false;

    if (!AreAllGroupMembersNearby(bot))
        return false;

    if (IsPortalNearby(bot, GetHomePortalKeyword(bot)))
        return false;

    return true;
}

bool WorldBuffTravelUsePortalTrigger::IsActive()
{
    uint8 step = AI_VALUE(uint8, "world buff travel step");
    if (step != static_cast<uint8>(WorldBuffTravelStep::STEP_PORTAL_HOME))
        return false;

    return IsPortalNearby(bot, GetHomePortalKeyword(bot));
}

bool WorldBuffTravelDoneTrigger::IsActive()
{
    uint8 step = AI_VALUE(uint8, "world buff travel step");
    return step >= static_cast<uint8>(WorldBuffTravelStep::STEP_DONE);
}