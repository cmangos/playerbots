#include "playerbot/playerbot.h"
#include "MonitorCombat.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;

bool MonitorCombatHp::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint8 hp = bot->GetHealthPercent();

    std::string valueName;
    std::string op;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, valueName, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == "<")
        return hp < threshold;

    return hp > threshold;
}

bool MonitorCombatMob::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    // monitorStr has "mob" prefix already stripped, e.g. " 11520 is dead => pass ..."
    size_t entryEnd = monitorStr.find("is dead");

    if (entryEnd == std::string::npos)
        return false;

    std::string entryStr = monitorStr.substr(0, entryEnd);
    uint32 entryId = atoi(entryStr.c_str());

    // Fast path: if focus mob tracking confirmed this entry dead
    if (ctx.focusMobKilled && ctx.focusMobEntry == entryId)
        return true;

    std::list<Creature*> creatures;
    MaNGOS::AnyUnitInObjectRangeCheck checker(bot, 500.0f);
    MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(creatures, checker);
    Cell::VisitWorldObjects(bot, searcher, 500.0f);

    bool found = false;
    bool allDead = true;

    for (auto& creature : creatures)
    {
        if (creature->GetEntry() == entryId)
        {
            found = true;
            if (creature->IsAlive())
                allDead = false;
        }
    }

    // Also check map store if not found nearby
    if (!found)
    {
        auto& objectStore = bot->GetMap()->GetObjectsStore();
        for (auto itr = objectStore.begin<Creature>(); itr != objectStore.end<Creature>(); ++itr)
        {
            if (Creature* c = itr->second)
            {
                if (c->GetEntry() == entryId)
                {
                    found = true;
                    if (c->IsAlive())
                        allDead = false;
                    break;
                }
            }
        }
    }

    if (found && allDead)   
        return true;

    return false;
}

bool MonitorCombatPartyWiped::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    uint32 aliveCount = 0;
    for (auto itr = group->GetMemberSlots().begin(); itr != group->GetMemberSlots().end(); ++itr)
    {
        Player* member = sObjectMgr.GetPlayer(itr->guid);
        if (member && member->IsAlive())
            aliveCount++;
    }

    if (aliveCount == 0)
        return true;

    return false;
}

bool MonitorCombatDeadMobs::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    // Sweep a wide radius around the bot and latch every dead creature GUID into ctx. A corpse only
    // needs to be observed once: bots loot kills almost immediately, so simultaneous-corpses counts
    // collapse to 0-2 in sparse-start instances even though the party is killing continuously.
    std::list<Creature*> creatures;
    // GetDistance(..., DIST_CALC_NONE) returns the SQUARED distance, so the checker range must be
    // squared too; VisitAllObjects keeps the linear radius for the cell visit.
    MaNGOS::AnyUnitFulfillingConditionInRangeCheck checker(bot, [](Unit* u) { return !u->IsAlive(); }, 300.0f * 300.0f, DIST_CALC_NONE);
    MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitFulfillingConditionInRangeCheck> searcher(creatures, checker);
    Cell::VisitAllObjects(bot, searcher, 300.0f);

    for (auto& creature : creatures)
    {
        if (!creature->IsAlive() && !creature->IsPet() && !creature->IsTotem())
            ctx.observedDeadMobs.insert(creature->GetObjectGuid());
    }

    uint32 deadCount = static_cast<uint32>(ctx.observedDeadMobs.size());

    std::string valueName;
    std::string op;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, valueName, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == ">")
        return deadCount > threshold;

    return deadCount < threshold;

}

bool MonitorCombatPartyXp::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    if (!ctx.partyXpCaptured)
        return false;

    uint64 current = GetPartyXpTotal(bot);
    uint64 start = ctx.partyXpStart;
    // Members can leave (or be logged out) mid-run and drop the summed total below the baseline.
    uint64 gained = current > start ? current - start : 0;

    std::string valueName;
    std::string op;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, valueName, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == ">")
        return gained > threshold;

    return gained < threshold;
}