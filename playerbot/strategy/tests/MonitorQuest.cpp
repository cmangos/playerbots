#include "playerbot/playerbot.h"
#include "MonitorQuest.h"
#include "Quests/QuestDef.h"

using namespace ai;

// =====================================================
// Helpers
// =====================================================
namespace
{
    uint32 ExtractLeadingId(const std::string& monitorStr)
    {
        size_t idStart = monitorStr.find_first_not_of(' ');
        if (idStart == std::string::npos)
            return 0;

        std::string idStr;
        size_t idEnd = monitorStr.find_first_of(" =><", idStart);
        if (idEnd == std::string::npos)
            idStr = monitorStr.substr(idStart);
        else
            idStr = monitorStr.substr(idStart, idEnd - idStart);

        char* endPtr = nullptr;
        unsigned long val = std::strtoul(idStr.c_str(), &endPtr, 10);
        if (endPtr == idStr.c_str() || val == 0)
            return 0;

        return static_cast<uint32>(val);
    }
}

// =====================================================
// MonitorQuestComplete
// Format: "quest complete <questId> => pass/fail \"message\""
// True when quest status is COMPLETE or has been rewarded
// =====================================================
bool MonitorQuestComplete::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint32 questId = ExtractLeadingId(monitorStr);
    if (!questId)
        return false;

    QuestStatus status = bot->GetQuestStatus(questId);
    if (status == QUEST_STATUS_COMPLETE)
        return true;

    // Also check rewarded
    auto it = bot->getQuestStatusMap().find(questId);
    if (it != bot->getQuestStatusMap().end() && it->second.m_rewarded)
        return true;

    return false;
}

// =====================================================
// MonitorQuestRewarded
// Format: "quest rewarded <questId> => pass/fail \"message\""
// True only when quest has been turned in and rewarded
// =====================================================
bool MonitorQuestRewarded::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint32 questId = ExtractLeadingId(monitorStr);
    if (!questId)
        return false;

    auto it = bot->getQuestStatusMap().find(questId);
    if (it != bot->getQuestStatusMap().end() && it->second.m_rewarded)
        return true;

    return false;
}

// =====================================================
// MonitorQuestActive
// Format: "quest active <questId> => pass/fail \"message\""
// True when bot has the quest in their log (incomplete or complete, not rewarded/none)
// =====================================================
bool MonitorQuestActive::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint32 questId = ExtractLeadingId(monitorStr);
    if (!questId)
        return false;

    QuestStatus status = bot->GetQuestStatus(questId);
    return (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE);
}

// =====================================================
// MonitorQuestObjective
// Format: "quest objective <questId> <objIndex> done => pass/fail \"message\""
// True when a specific objective (0-3) is fully satisfied
// =====================================================
bool MonitorQuestObjective::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    size_t idStart = monitorStr.find_first_not_of(' ');
    if (idStart == std::string::npos)
        return false;

    size_t idEnd = monitorStr.find(' ', idStart);
    if (idEnd == std::string::npos)
        return false;

    std::string idStr = monitorStr.substr(idStart, idEnd - idStart);
    uint32 questId = static_cast<uint32>(std::strtoul(idStr.c_str(), nullptr, 10));
    if (!questId)
        return false;

    // Extract objective index
    size_t objStart = monitorStr.find_first_not_of(' ', idEnd);
    if (objStart == std::string::npos)
        return false;

    size_t objEnd = monitorStr.find_first_of(" =><", objStart);
    std::string objStr;
    if (objEnd == std::string::npos)
        objStr = monitorStr.substr(objStart);
    else
        objStr = monitorStr.substr(objStart, objEnd - objStart);

    uint32 objIndex = static_cast<uint32>(std::strtoul(objStr.c_str(), nullptr, 10));
    if (objIndex >= QUEST_OBJECTIVES_COUNT)
        return false;

    Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
    if (!quest)
        return false;

    auto it = bot->getQuestStatusMap().find(questId);
    if (it == bot->getQuestStatusMap().end())
        return false;

    const QuestStatusData& statusData = it->second;

    // Check creature/GO count objective
    if (quest->ReqCreatureOrGOId[objIndex])
    {
        return statusData.m_creatureOrGOcount[objIndex] >= quest->ReqCreatureOrGOCount[objIndex];
    }

    // Check item count objective (if objIndex < QUEST_ITEM_OBJECTIVES_COUNT)
    if (objIndex < QUEST_ITEM_OBJECTIVES_COUNT && quest->ReqItemId[objIndex])
    {
        uint32 itemCount = bot->GetItemCount(quest->ReqItemId[objIndex], true);
        return itemCount >= quest->ReqItemCount[objIndex];
    }

    return false;
}

// =====================================================
// MonitorHasItem
// Format: "has item <itemId> => pass/fail \"message\""
// True when bot has at least 1 of the item
// =====================================================
bool MonitorHasItem::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint32 itemId = ExtractLeadingId(monitorStr);
    if (!itemId)
        return false;

    return bot->GetItemCount(itemId, true) > 0;
}

// =====================================================
// MonitorOnMap
// Format: "on map <mapId> => pass/fail \"message\""
// True when bot is on the specified map
// =====================================================
bool MonitorOnMap::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    uint32 mapId = ExtractLeadingId(monitorStr);
    if (!mapId && monitorStr.find(" 0 ") == std::string::npos) // map 0 is valid
        return false;

    return bot->GetMapId() == mapId;
}

// =====================================================
// MonitorHasMount
// Format: "has mount => pass/fail \"message\""
// True when bot knows a mount spell
// =====================================================
bool MonitorHasMount::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    // Check common DK mount spell (Acherus Deathcharger)
    // SpellID 48778 = Acherus Deathcharger
    if (bot->HasSpell(48778))
        return true;

    // Check generic mount spells - any riding skill mount
    const PlayerSpellMap& spellMap = bot->GetSpellMap();
    for (const auto& pair : spellMap)
    {
        if (pair.second.state == PLAYERSPELL_REMOVED)
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(pair.first);
        if (!spellInfo)
            continue;

        // Check if it's a mount spell (has SPELL_AURA_MOUNTED effect)
        for (int i = 0; i < 3; ++i)
        {
            if (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOUNTED)
                return true;
        }
    }

    return false;
}
