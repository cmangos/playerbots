#include "playerbot/playerbot.h"
#include "CommandQuest.h"
#include "Quests/QuestDef.h"
#include "Globals/ObjectMgr.h"

using namespace ai;

// =====================================================
// CommandSetupAcceptQuest
// Format: "accept quest <questId>"
// =====================================================
TestResult CommandSetupAcceptQuest::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    std::string questStr = params;
    // Strip "quest " prefix if present from params
    if (questStr.find("quest ") == 0)
        questStr = questStr.substr(6);

    uint32 questId = static_cast<uint32>(std::strtoul(questStr.c_str(), nullptr, 10));
    if (!questId)
    {
        message = "Invalid quest ID: " + questStr;
        return TestResult::IMPOSSIBLE;
    }

    Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
    if (!quest)
    {
        message = "Quest template not found: " + questStr;
        return TestResult::IMPOSSIBLE;
    }

    // Check if already has this quest
    QuestStatus status = bot->GetQuestStatus(questId);
    if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE)
        return TestResult::PASS; // Already have it

    if (!bot->CanAddQuest(quest, false))
    {
        message = "Bot cannot accept quest " + questStr + " (log full or requirements not met)";
        return TestResult::IMPOSSIBLE;
    }

    bot->AddQuest(quest, nullptr);
    bot->GiveQuestSourceItemIfNeed(quest);

    if (uint32 sourceItemId = quest->GetSrcItemId())
    {
        uint32 sourceItemCount = std::max<uint32>(1, quest->GetSrcItemCount());
        if (bot->GetItemCount(sourceItemId, true) < sourceItemCount)
        {
            message = "Quest " + questStr + " accepted but missing required source item " + std::to_string(sourceItemId);
            return TestResult::IMPOSSIBLE;
        }
    }

    return TestResult::PASS;
}

// =====================================================
// CommandSetupForceCompleteQuest
// Format: "complete quest <questId>"
// Force-completes all objectives (items + creature/GO counts)
// =====================================================
TestResult CommandSetupForceCompleteQuest::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    std::string questStr = params;
    if (questStr.find("quest ") == 0)
        questStr = questStr.substr(6);

    uint32 questId = static_cast<uint32>(std::strtoul(questStr.c_str(), nullptr, 10));
    if (!questId)
    {
        message = "Invalid quest ID: " + questStr;
        return TestResult::IMPOSSIBLE;
    }

    Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
    if (!quest)
    {
        message = "Quest template not found: " + questStr;
        return TestResult::IMPOSSIBLE;
    }

    QuestStatus status = bot->GetQuestStatus(questId);
    if (status == QUEST_STATUS_NONE)
    {
        message = "Bot does not have quest " + questStr;
        return TestResult::IMPOSSIBLE;
    }

    if (status == QUEST_STATUS_COMPLETE)
        return TestResult::PASS;

    // Satisfy item objectives
    for (uint8 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
    {
        uint32 itemId = quest->ReqItemId[i];
        uint32 count = quest->ReqItemCount[i];
        if (!itemId || !count)
            continue;

        uint32 curCount = bot->GetItemCount(itemId, true);
        if (curCount >= count)
            continue;

        ItemPosCountVec dest;
        uint8 msg = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count - curCount);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = bot->StoreNewItem(dest, itemId, true);
            bot->SendNewItem(item, count - curCount, true, false);
        }
    }

    // Satisfy creature/GO kill objectives
    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        int32 creature = quest->ReqCreatureOrGOId[i];
        uint32 creatureCount = quest->ReqCreatureOrGOCount[i];
        if (!creature || !creatureCount)
            continue;

        if (uint32 spellId = quest->ReqSpell[i])
        {
            for (uint32 z = 0; z < creatureCount; ++z)
                bot->CastedCreatureOrGO(creature, ObjectGuid((creature > 0 ? HIGHGUID_UNIT : HIGHGUID_GAMEOBJECT), uint32(std::abs(creature)), 1u), spellId);
        }
        else if (creature > 0)
        {
            CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature);
            if (cInfo)
            {
                for (uint32 z = 0; z < creatureCount; ++z)
                    bot->KilledMonster(cInfo, nullptr);
            }
        }
        else if (creature < 0)
        {
            for (uint32 z = 0; z < creatureCount; ++z)
                bot->CastedCreatureOrGO(creature, ObjectGuid(HIGHGUID_GAMEOBJECT, uint32(std::abs(creature)), 1u), 0);
        }
    }

    return TestResult::PASS;
}

// =====================================================
// CommandSetupRewardQuest
// Format: "reward quest <questId>"
// Completes objectives and rewards the quest immediately
// =====================================================
TestResult CommandSetupRewardQuest::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    std::string questStr = params;
    if (questStr.find("quest ") == 0)
        questStr = questStr.substr(6);

    uint32 questId = static_cast<uint32>(std::strtoul(questStr.c_str(), nullptr, 10));
    if (!questId)
    {
        message = "Invalid quest ID: " + questStr;
        return TestResult::IMPOSSIBLE;
    }

    Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
    if (!quest)
    {
        message = "Quest template not found: " + questStr;
        return TestResult::IMPOSSIBLE;
    }

    // Check if already rewarded
    auto it = bot->getQuestStatusMap().find(questId);
    if (it != bot->getQuestStatusMap().end() && it->second.m_rewarded)
        return TestResult::PASS;

    // Accept if not already
    QuestStatus status = bot->GetQuestStatus(questId);
    if (status == QUEST_STATUS_NONE)
    {
        if (!bot->CanAddQuest(quest, false))
        {
            message = "Bot cannot accept quest " + questStr;
            return TestResult::IMPOSSIBLE;
        }
        bot->AddQuest(quest, nullptr);
    }

    // Satisfy all objectives
    for (uint8 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
    {
        uint32 itemId = quest->ReqItemId[i];
        uint32 count = quest->ReqItemCount[i];
        if (!itemId || !count)
            continue;

        uint32 curCount = bot->GetItemCount(itemId, true);
        if (curCount >= count)
            continue;

        ItemPosCountVec dest;
        uint8 msg = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count - curCount);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = bot->StoreNewItem(dest, itemId, true);
            bot->SendNewItem(item, count - curCount, true, false);
        }
    }

    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        int32 creature = quest->ReqCreatureOrGOId[i];
        uint32 creatureCount = quest->ReqCreatureOrGOCount[i];
        if (!creature || !creatureCount)
            continue;

        if (uint32 spellId = quest->ReqSpell[i])
        {
            for (uint32 z = 0; z < creatureCount; ++z)
                bot->CastedCreatureOrGO(creature, ObjectGuid((creature > 0 ? HIGHGUID_UNIT : HIGHGUID_GAMEOBJECT), uint32(std::abs(creature)), 1u), spellId);
        }
        else if (creature > 0)
        {
            CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature);
            if (cInfo)
                for (uint32 z = 0; z < creatureCount; ++z)
                    bot->KilledMonster(cInfo, nullptr);
        }
        else if (creature < 0)
        {
            for (uint32 z = 0; z < creatureCount; ++z)
                bot->CastedCreatureOrGO(creature, ObjectGuid(HIGHGUID_GAMEOBJECT, uint32(std::abs(creature)), 1u), 0);
        }
    }

    // Reward the quest
    bot->RewardQuest(quest, 0, bot, false);

    return TestResult::PASS;
}

// =====================================================
// CommandSetupDo
// Format: "do <chat command>"
// Executes a bot chat command (like equip upgrades, accept quest, etc.)
// =====================================================
TestResult CommandSetupDo::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    if (params.empty())
    {
        message = "No command specified for 'do'";
        return TestResult::IMPOSSIBLE;
    }

    ai->HandleCommand(ChatMsg::CHAT_MSG_WHISPER, params, *bot);
    return TestResult::PASS;
}
