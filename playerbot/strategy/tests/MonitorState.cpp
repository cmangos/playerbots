#include "playerbot/playerbot.h"
#include "MonitorState.h"
#include "playerbot/WorldPosition.h"
#include "playerbot/ChatHelper.h"
#include "Server/DBCStores.h"
#include <cctype>
#include <set>

using namespace ai;

bool MonitorStateTime::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint32 elapsed = 0;
    if (!ctx.monitorTime)
        ctx.monitorTime = WorldTimer::getMSTime();
    else
        elapsed = (WorldTimer::getMSTime() - ctx.monitorTime) / 1000;

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
        return size > threshold;

    return size < threshold;
}

bool MonitorStateLootGuid::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint64 lootGuid = bot->GetLootGuid().GetRawValue();

    std::string valueName;
    std::string op;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, valueName, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint64 threshold = 0;
    if (TryParseUInt64Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == ">")
        return lootGuid > threshold;

    return lootGuid < threshold;
}

bool MonitorStateStarterGearCount::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    std::string leftSide;
    std::string rightSide;
    std::string parseMessage;
    if (TrySplitOnce(monitorStr, "=>", leftSide, rightSide, parseMessage, GetName(), true) != TestResult::PASS)
        return false;

    std::string valueName;
    std::string op;
    std::string valueStr;
    if (TryParseComparisonValue(monitorStr, valueName, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 packed = bot->GetUInt32Value(UNIT_FIELD_BYTES_0) & 0x00FFFFFF;
    std::set<uint32> starterItems;
    for (uint32 i = 0; i < sCharStartOutfitStore.GetNumRows(); ++i)
    {
        CharStartOutfitEntry const* entry = sCharStartOutfitStore.LookupEntry(i);
        if (!entry || entry->RaceClassGender != packed)
            continue;

        for (int32 itemId : entry->ItemId)
        {
            if (itemId > 0)
                starterItems.insert(static_cast<uint32>(itemId));
        }
        break;
    }

    uint32 count = 0;
    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
            continue;

        ItemPrototype const* proto = item->GetProto();
        if (!proto)
            continue;

        if (starterItems.find(proto->ItemId) != starterItems.end())
            ++count;
    }

    if (op == ">")
        return count > threshold;

    return count < threshold;
}

bool MonitorStateEquipQuality::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    std::string leftSide;
    std::string rightSide;
    std::string parseMessage;
    if (TrySplitOnce(monitorStr, "=>", leftSide, rightSide, parseMessage, GetName(), true) != TestResult::PASS)
        return false;

    std::string qualityAndCmp = leftSide;
    
    size_t ltPos = qualityAndCmp.find('<');
    size_t gtPos = qualityAndCmp.find('>');
    size_t opPos = std::string::npos;
    std::string op;

    if (ltPos != std::string::npos && (gtPos == std::string::npos || ltPos < gtPos))
    {
        opPos = ltPos;
        op = '<';
    }
    else if (gtPos != std::string::npos)
    {
        opPos = gtPos;
        op = '>';
    }
    else
    {
        return false;
    }

    std::string qualityName = qualityAndCmp.substr(0, opPos);
    while (!qualityName.empty() && std::isspace(static_cast<unsigned char>(qualityName.back())))
        qualityName.pop_back();

    uint32 quality = ChatHelper::parseItemQuality(qualityName);
    if (quality == MAX_ITEM_QUALITY)
        return false;

    std::string valueStr = qualityAndCmp.substr(opPos + 1);
    while (!valueStr.empty() && std::isspace(static_cast<unsigned char>(valueStr[0])))
        valueStr.erase(valueStr.begin());

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 count = 0;
    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
            continue;

        ItemPrototype const* proto = item->GetProto();
        if (!proto)
            continue;

        if (proto->Quality == quality)
            ++count;
    }

    if (op == ">")
        return count > threshold;

    return count < threshold;
}

bool MonitorStateAreaLevelDiff::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    std::string op;
    std::string valueName;
    std::string valueStr;
    std::string parseMessage;
    if (TryParseComparisonValue(monitorStr, valueName, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 threshold = 0;
    if (TryParseUInt32Strict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    uint32 areaLevel = WorldPosition(bot).getAreaLevel();
    if (!areaLevel)
        return true;

    uint32 botLevel = bot->GetLevel();
    uint32 diff = areaLevel > botLevel ? (areaLevel - botLevel) : (botLevel - areaLevel);

    if (op == ">")
        return diff > threshold;

    return diff < threshold;
}

bool MonitorAiValue::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    //monitor value uint32 item count::2313 > 0 => pass \"Item withdrawn to inventory\"
    AiObjectContext* context = bot->GetPlayerbotAI()->GetAiObjectContext();
    std::string valueStr;
    std::string valueToCompareTo;
    std::string parseMessage;
    std::string op;
    if (TryParseComparisonValue(monitorStr, valueStr, op, valueToCompareTo, parseMessage, GetName()) != TestResult::PASS)
        return false;

    const size_t firstSpace = valueStr.find(' ');
    if (firstSpace == std::string::npos)
        return false;

    std::string datatype = valueStr.substr(0, firstSpace);
    std::string valueName = valueStr.substr(firstSpace + 1);

    if (datatype == "uint32")
    {
        uint32 count = AI_VALUE(uint32, valueName);

        uint32 threshold = 0;
        if (TryParseUInt32Strict(valueToCompareTo, threshold, parseMessage, GetName()) != TestResult::PASS)
            return false;

        if(op == "==") return count == threshold; // == case
        if(op == "!=") return count != threshold; // != case
        if(op == "<") return count < threshold;
        if(op == ">") return count > threshold;
        return false;
    }
    else if (datatype == "bool")
    {
        bool val = AI_VALUE(bool, valueName);

        bool threshold = (valueToCompareTo == "true");
        if(op == "==") return val == threshold;
        if(op == "!=") return val != threshold;
        return false;
    }

    return false;
}

bool MonitorOutgoingMessage::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    std::string leftSide, rightSide, parseMessage;
    const TestResult splitResult = TrySplitOnce(monitorStr, "=>", leftSide, rightSide, parseMessage, GetName(), true);
    if (splitResult != TestResult::PASS)
        return false;
    
    PlayerbotAI* ai = bot->GetPlayerbotAI();

    bool isFound = false;

    for(auto& msg : ai->GetRecordedMessages())
    {
        if (leftSide == "*" || msg.find(leftSide) != std::string::npos)
            isFound = true;

        if (ctx.debug)
        {
            sLog.outString("[TestAction] Bot %s outgoing %s", bot->GetName(), msg.c_str());
        }
    }

    ai->RecordMessages(true); // Stop recording messages after checking

    return isFound;
}