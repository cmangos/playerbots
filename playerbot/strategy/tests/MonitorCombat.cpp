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
    std::list<Creature*> creatures;
    MaNGOS::AnyUnitInObjectRangeCheck checker(bot, 120.0f);
    MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(creatures, checker);
    Cell::VisitWorldObjects(bot, searcher, 120.0f);

    uint32 deadCount = 0;
    for (auto& creature : creatures)
    {
        if (!creature->IsAlive())
            deadCount++;
    }

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