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

// Every group member has been observed on the bot's map, in its instance, and settled (not
// mid-teleport). Used to prove a cross-map group teleport actually delivered the members, rather than
// merely not tripping an assert. "group size" cannot serve as a pass condition for that: it is already
// satisfied the moment the group exists, so it would pass before the teleport was issued.
//
// Delivery is recorded **per member** (in TestContext) rather than as one simultaneous snapshot: group
// members are roamed random bots and are rarely all settled on the host's map on the same tick. Each
// member is latched the first time it is seen there, and the condition completes once every member slot
// has been seen at least once - the assertion is that the delivery happened, not that it held.
//
// "group on map <mapId|mapName>" additionally pins the bot itself, which is what makes the
// condition usable as a pass condition for a cross-map move: without the pin the check is already
// true while the whole group still sits on the map it was formed on.
bool MonitorStateGroupOnMap::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // GetFirstMember() walks live GroupReference links, so an offline member is invisible there while
    // GetMembersCount() still counts its slot - hence the slot count rather than a live-member count.
    const uint32 expectedMembers = group->GetMembersCount() > 0 ? group->GetMembersCount() - 1 : 0;

    // Already complete: sticky, so later roaming cannot un-prove a delivery that was observed.
    if (expectedMembers > 0 && ctx.groupMembersSeenOnMap.size() >= expectedMembers)
        return true;

    // Optional map gate, e.g. "1" in "monitor group on map 1 => pass ...". The framework hands this
    // function the monitor line with the monitor name **already removed** (TestAction::CheckMonitors
    // does monitorStr.substr(GetNameSize())), so the gate is simply the text before "=>". Searching
    // for the name in here can never match and would make the condition permanently false.
    std::string param;
    size_t paramEnd = monitorStr.find("=>");
    param = paramEnd == std::string::npos ? monitorStr : monitorStr.substr(0, paramEnd);
    while (!param.empty() && (param.front() == ' ' || param.front() == '\t'))
        param.erase(param.begin());
    while (!param.empty() && (param.back() == ' ' || param.back() == '\t'))
        param.pop_back();

    const uint32 mapId = bot->GetMapId();
    const uint32 instanceId = bot->GetInstanceId();

    if (!param.empty())
    {
        uint32 requiredMapId = 0;
        std::string parseMessage;
        if (TryParseUInt32Strict(param, requiredMapId, parseMessage, GetName()) == TestResult::PASS)
        {
            if (mapId != requiredMapId)
                return false;
        }
        else
        {
            // Compares the *map* name (e.g. "kalimdor"), not the zone or area name - "orgrimmar" is an
            // area inside Kalimdor and would simply never match. Prefer the numeric map id when writing
            // a test; a typo here degrades to a condition that is never true, not to a false pass.
            WorldPosition botPos(bot);
            const MapEntry* entry = botPos ? botPos.getMapEntry() : nullptr;
            std::string have = (entry && entry->name[0]) ? entry->name[0] : "";
            std::string want = param;
            for (char& c : have)
                c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            for (char& c : want)
                c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            if (have != want)
                return false;
        }
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || member == bot)
            continue;

        // Skip rather than fail when the member is not there yet: a staggered landing is still a
        // delivery, and it is recorded on the tick the member finally settles.
        if (!member->IsInWorld() || member->IsBeingTeleported())
            continue;

        // GetMapId()/GetInstanceId() read cached members, so unlike GetMap() they cannot assert.
        if (member->GetMapId() != mapId || member->GetInstanceId() != instanceId)
            continue;

        ctx.groupMembersSeenOnMap.insert(member->GetObjectGuid());
    }

    return expectedMembers > 0 && ctx.groupMembersSeenOnMap.size() >= expectedMembers;
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