#include "playerbot/playerbot.h"
#include "MonitorCombat.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;

bool MonitorCombatHp::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint8 hp = bot->GetHealthPercent();

    size_t ltPos = monitorStr.find("<");
    size_t gtPos = monitorStr.find(">");
    size_t arrowPos = monitorStr.find("=>");

    if (ltPos != std::string::npos && arrowPos != std::string::npos)
    {
        std::string percentStr = monitorStr.substr(ltPos + 1, arrowPos - ltPos - 1);
        uint32 threshold = atoi(percentStr.c_str());

        if (hp < threshold)
        {
            return true;
        }
    }
    else if (gtPos != std::string::npos && arrowPos != std::string::npos)
    {
        std::string percentStr = monitorStr.substr(gtPos + 1, arrowPos - gtPos - 1);
        uint32 threshold = atoi(percentStr.c_str());
        if (hp > threshold)
        {
            return true;
        }
    }
    return false;
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
    size_t arrowPos = monitorStr.find("=>");
    if (arrowPos == std::string::npos)
        return false;

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

    size_t gtPos = monitorStr.find(">");
    if (gtPos != std::string::npos)
    {
        uint32 threshold = atoi(monitorStr.substr(gtPos + 1, arrowPos - gtPos - 1).c_str());
        return deadCount > threshold;
    }

    size_t ltPos = monitorStr.find("<");
    if (ltPos != std::string::npos)
    {
        uint32 threshold = atoi(monitorStr.substr(ltPos + 1, arrowPos - ltPos - 1).c_str());
        return deadCount < threshold;
    }

    return false;
}