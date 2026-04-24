#include "playerbot/playerbot.h"
#include "MonitorState.h"

using namespace ai;

bool MonitorStateTime::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint32 elapsed = (WorldTimer::getMSTime() - ctx.testStartTime) / 1000;

    char op = 0;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == '>')
        return elapsed >= threshold;

    return elapsed <= threshold;
}

bool MonitorStateDead::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    return !bot->IsAlive();
}

bool MonitorStateFaction::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    size_t arrowPos = monitorStr.find("=>");
    std::string faction = monitorStr.substr(GetName().length() + 1, arrowPos - GetName().length()-2);

    if (faction == "horde")
        return bot->GetTeam() == HORDE;
    else if (faction == "alliance")
        return bot->GetTeam() == ALLIANCE;

    return false;
}

bool MonitorStateGroupSize::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    Group* group = bot->GetGroup();
    uint32 size = group ? group->GetMembersCount() : 1;

    char op = 0;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == '>')
        return size > threshold;

    return size < threshold;
}

bool MonitorStateLootGuid::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint64 lootGuid = bot->GetLootGuid().GetRawValue();

    char op = 0;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint64 threshold = 0;
    if (TryParseUInt64Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == '>')
        return lootGuid > threshold;

    return lootGuid < threshold;
}