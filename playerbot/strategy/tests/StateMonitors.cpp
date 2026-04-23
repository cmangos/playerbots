#include "playerbot/playerbot.h"
#include "StateMonitors.h"

using namespace ai;

bool CheckTimeMonitor::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint32 elapsed = (WorldTimer::getMSTime() - ctx.testStartTime) / 1000;

    size_t gtPos = monitorStr.find(">");
    size_t arrowPos = monitorStr.find("=>");

    if (gtPos != std::string::npos && arrowPos != std::string::npos)
    {
        std::string secondsStr = monitorStr.substr(gtPos+2, arrowPos - gtPos - 1);
        uint32 threshold = atoi(secondsStr.c_str());

        if (elapsed >= threshold)
        {
            return true;
        }
    }
    return false;
}

bool BotDeadMonitor::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    return !bot->IsAlive();
}

bool FactionMonitor::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    size_t arrowPos = monitorStr.find("=>");
    std::string faction = monitorStr.substr(GetName().length() + 1, arrowPos - GetName().length()-2);

    if (faction == "horde")
        return bot->GetTeam() == HORDE;
    else if (faction == "alliance")
        return bot->GetTeam() == ALLIANCE;

    return false;
}

bool GroupSizeMonitor::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    Group* group = bot->GetGroup();
    uint32 size = group ? group->GetMembersCount() : 1;

    size_t arrowPos = monitorStr.find("=>");
    if (arrowPos == std::string::npos)
        return false;

    size_t gtPos = monitorStr.find(">");
    if (gtPos != std::string::npos)
    {
        uint32 threshold = atoi(monitorStr.substr(gtPos + 1, arrowPos - gtPos - 1).c_str());
        return size > threshold;
    }

    size_t ltPos = monitorStr.find("<");
    if (ltPos != std::string::npos)
    {
        uint32 threshold = atoi(monitorStr.substr(ltPos + 1, arrowPos - ltPos - 1).c_str());
        return size < threshold;
    }

    return false;
}

bool LootGuidMonitor::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint64 lootGuid = bot->GetLootGuid().GetRawValue();

    size_t arrowPos = monitorStr.find("=>");
    if (arrowPos == std::string::npos)
        return false;

    size_t gtPos = monitorStr.find(">");
    if (gtPos != std::string::npos)
    {
        uint64 threshold = static_cast<uint64>(atoll(monitorStr.substr(gtPos + 1, arrowPos - gtPos - 1).c_str()));
        return lootGuid > threshold;
    }

    size_t ltPos = monitorStr.find("<");
    if (ltPos != std::string::npos)
    {
        uint64 threshold = static_cast<uint64>(atoll(monitorStr.substr(ltPos + 1, arrowPos - ltPos - 1).c_str()));
        return lootGuid < threshold;
    }

    return false;
}