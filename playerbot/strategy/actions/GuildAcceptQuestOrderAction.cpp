
#include "playerbot/playerbot.h"
#include "GuildAcceptQuestOrderAction.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool GuildAcceptQuestOrderAction::isUseful()
{
    if (!bot->GetGuildId())
        return false;

    if (bot->IsInCombat())
        return false;

    return AI_VALUE(bool, "needs guild quest order accept");
}

bool GuildAcceptQuestOrderAction::Execute(Event& event)
{
    GuildOrder order = AI_VALUE(GuildOrder, "guild order");
    if (!order.IsQuestRewardOrder() || !order.questId)
        return false;

    Quest const* quest = sObjectMgr.GetQuestTemplate(order.questId);
    if (!quest)
        return false;

    QuestStatus status = bot->GetQuestStatus(order.questId);
    if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE)
        return false; // Already have it.

    if (!bot->CanTakeQuest(quest, false))
        return false;

    if (!bot->SatisfyQuestLog(false))
    {
        std::vector<std::pair<uint8, uint32>> candidates;

        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 questId = bot->GetQuestSlotQuestId(slot);
            if (!questId)
                continue;

            if (questId == order.questId)
                continue;

            Quest const* candidate = sObjectMgr.GetQuestTemplate(questId);
            if (!candidate)
                continue;

            if (candidate->GetRequiredClasses())
                continue;

            candidates.push_back({ slot, questId });
        }

        if (candidates.empty())
        {
            ai->TellDebug(ai->GetMaster(), "Quest log full but no droppable quests found for guild order", "debug travel");
            return false;
        }

        std::vector<std::pair<uint8, uint32>> noProgressCandidates;
        for (auto& [slot, questId] : candidates)
        {
            Quest const* candidate = sObjectMgr.GetQuestTemplate(questId);
            if (candidate && bot->GetQuestStatus(questId) != QUEST_STATUS_COMPLETE)
            {
                QuestStatusData& qsd = bot->getQuestStatusMap()[questId];
                bool hasProgress = false;
                for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if (candidate->ReqItemId[i] && qsd.m_itemcount[i] > 0)
                    {
                        hasProgress = true; break;
                    }
                    if (candidate->ReqCreatureOrGOId[i] && qsd.m_creatureOrGOcount[i] > 0)
                    {
                        hasProgress = true; break;
                    }
                }
                if (!hasProgress)
                    noProgressCandidates.push_back({ slot, questId });
            }
        }

        auto& dropPool = noProgressCandidates.empty() ? candidates : noProgressCandidates;
        auto& [dropSlot, dropQuestId] = dropPool[urand(0, dropPool.size() - 1)];

        Quest const* droppedQuest = sObjectMgr.GetQuestTemplate(dropQuestId);
        ai->TellDebug(ai->GetMaster(), "Dropping quest [" + (droppedQuest ? droppedQuest->GetTitle() : std::to_string(dropQuestId)) + "] to make room for guild order quest", "debug travel");
        ai->DropQuest(dropQuestId);

        if (!bot->SatisfyQuestLog(false))
            return false;
    }

    // Search nearby NPCs for one that offers this quest.
    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || bot->GetDistance(unit) > INTERACTION_DISTANCE)
            continue;

        if (!unit->HasQuest(order.questId))
            continue;

        if (!sServerFacade.IsInFront(bot, unit, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT))
            sServerFacade.SetFacingTo(bot, unit);

        // Accept the quest via the opcode handler.
        WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
        uint32 unk1 = 0;
        p << unit->GetObjectGuid() << order.questId << unk1;
        p.rpos(0);
        bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(p);

        if (bot->GetQuestStatus(order.questId) != QUEST_STATUS_NONE &&
            bot->GetQuestStatus(order.questId) != QUEST_STATUS_AVAILABLE)
        {
            ai->TellDebug(ai->GetMaster(), "Guild quest order: accepted quest " + order.target, "debug travel");

            if (sPlayerbotAIConfig.globalSoundEffects)
                bot->PlayDistanceSound(620);

            return true;
        }
    }

    // Also check nearby game objects (some quests come from objects).
    std::list<ObjectGuid> gos = AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");
    for (auto& guid : gos)
    {
        GameObject* go = ai->GetGameObject(guid);
        if (!go || bot->GetDistance(go) > INTERACTION_DISTANCE)
            continue;

        if (!go->HasQuest(order.questId))
            continue;

        WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
        uint32 unk1 = 0;
        p << go->GetObjectGuid() << order.questId << unk1;
        p.rpos(0);
        bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(p);

        if (bot->GetQuestStatus(order.questId) != QUEST_STATUS_NONE &&
            bot->GetQuestStatus(order.questId) != QUEST_STATUS_AVAILABLE)
        {
            ai->TellDebug(ai->GetMaster(), "Guild quest order: accepted quest " + order.target, "debug travel");

            if (sPlayerbotAIConfig.globalSoundEffects)
                bot->PlayDistanceSound(620);

            return true;
        }
    }

    return false;
}