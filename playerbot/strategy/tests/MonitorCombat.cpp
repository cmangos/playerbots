#include "playerbot/playerbot.h"
#include "MonitorCombat.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;

bool MonitorCombatHp::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint8 hp = bot->GetHealthPercent();

    char op = 0;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == '<')
        return hp < threshold;

    return hp > threshold;
}

bool MonitorCombatMob::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    size_t entryStart = monitorStr.find("mob") + 3;
    size_t entryEnd = monitorStr.find("is dead");

    if (entryEnd == std::string::npos)
        return false;

    std::string entryStr = monitorStr.substr(entryStart, entryEnd - entryStart);
    uint32 entryId = atoi(entryStr.c_str());

    std::list<Creature*> creatures;
    MaNGOS::AnyUnitInObjectRangeCheck checker(bot, 100.0f);
    MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(creatures, checker);
    Cell::VisitWorldObjects(bot, searcher, 100.0f);

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

    char op = 0;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == '>')
        return deadCount > threshold;

    return deadCount < threshold;

}