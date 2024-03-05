
#include "playerbot/playerbot.h"
#include "ListQuestsActions.h"
#include "playerbot/TravelMgr.h"

using namespace ai;

bool ListQuestsAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (event.getParam() == "completed" || event.getParam() == "co")
    {
        ListQuests(requester, QUEST_LIST_FILTER_COMPLETED);
    }
    else if (event.getParam() == "incompleted" || event.getParam() == "in")
    {
        ListQuests(requester, QUEST_LIST_FILTER_INCOMPLETED);
    }
    else if (event.getParam() == "all")
    {
        ListQuests(requester, QUEST_LIST_FILTER_ALL);
    }
    else if (event.getParam() == "travel")
    {
        ListQuests(requester, QUEST_LIST_FILTER_ALL, QUEST_TRAVEL_DETAIL_SUMMARY);
    }
    else if (event.getParam() == "travel detail")
    {
        ListQuests(requester, QUEST_LIST_FILTER_ALL, QUEST_TRAVEL_DETAIL_FULL);
    }
    else
    {
        ListQuests(requester, QUEST_LIST_FILTER_SUMMARY);
    }
    return true;
}

void ListQuestsAction::ListQuests(Player* requester, QuestListFilter filter, QuestTravelDetail travelDetail)
{
    bool showIncompleted = filter & QUEST_LIST_FILTER_INCOMPLETED;
    bool showCompleted = filter & QUEST_LIST_FILTER_COMPLETED;

    if (showIncompleted)
        ai->TellPlayer(requester, "--- Incompleted quests ---");
    int incompleteCount = ListQuests(requester, false, !showIncompleted, travelDetail);

    if (showCompleted)
        ai->TellPlayer(requester, "--- Completed quests ---");
    int completeCount = ListQuests(requester, true, !showCompleted, travelDetail);

    ai->TellPlayer(requester, "--- Summary ---");
    std::ostringstream out;
    out << "Total: " << (completeCount + incompleteCount) << " / 25 (incompleted: " << incompleteCount << ", completed: " << completeCount << ")";
    ai->TellPlayer(requester, out);
}

int ListQuestsAction::ListQuests(Player* requester, bool completed, bool silent, QuestTravelDetail travelDetail)
{
    TravelTarget* target;
    WorldPosition botPos(bot);
    
    if (travelDetail != QUEST_TRAVEL_DETAIL_NONE)
        target = context->GetValue<TravelTarget*>("travel target")->Get();

    int count = 0;
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);
        bool isCompletedQuest = bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE;
        if (completed != isCompletedQuest)
            continue;

        count++;

        if (silent)
            continue;

        ai->TellPlayer(requester, chat->formatQuest(pQuest));

        if (travelDetail != QUEST_TRAVEL_DETAIL_NONE && target->getDestination())
        {
            if (target->getDestination()->getName() == "QuestRelationTravelDestination" || target->getDestination()->getName() == "QuestObjectiveTravelDestination")
            {
                QuestTravelDestination* QuestDestination = (QuestTravelDestination*)target->getDestination();

                if (QuestDestination->GetQuestTemplate()->GetQuestId() == questId)
                {
                    std::ostringstream out;

                    out << "[Active] traveling " << target->getPosition()->distance(botPos);

                    out << " to " << QuestDestination->getTitle();

                    ai->TellPlayer(requester, out);
                }
            }
        }

        if (travelDetail == QUEST_TRAVEL_DETAIL_SUMMARY)
        {
            std::vector<TravelDestination*> allDestinations = sTravelMgr.getQuestTravelDestinations(bot, questId, true, true, -1);
            std::vector<TravelDestination*> availDestinations = sTravelMgr.getQuestTravelDestinations(bot, questId, ai->GetMaster(), false, -1);

            uint32 desTot = allDestinations.size();
            uint32 desAvail = availDestinations.size();
            uint32 desFull = desAvail - sTravelMgr.getQuestTravelDestinations(bot, questId, false, false, -1).size();
            uint32 desRange = desAvail - sTravelMgr.getQuestTravelDestinations(bot, questId, false, false).size();

            uint32 tpoints = 0;
            uint32 apoints = 0;

            for (auto dest : allDestinations)
                tpoints += dest->getPoints(true).size();

            for (auto dest : availDestinations)
                apoints += dest->getPoints().size();

            std::ostringstream out;

            out << desAvail << "/" << desTot << " destinations " << apoints << "/" << tpoints << " points. ";
            if (desFull > 0)
                out << desFull << " crowded.";
            if (desRange > 0)
                out << desRange << " out of range.";

            ai->TellPlayer(requester, out);
        }
        else if (travelDetail == QUEST_TRAVEL_DETAIL_FULL)
        {
            uint32 limit = 0;
            std::vector<TravelDestination*> allDestinations = sTravelMgr.getQuestTravelDestinations(bot, questId, true, true, -1);

            std::sort(allDestinations.begin(), allDestinations.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->distanceTo(botPos) < j->distanceTo(botPos); });

            for (auto dest : allDestinations)
            {
                if (limit > 5)
                    continue;

                std::ostringstream out;

                uint32 tpoints = dest->getPoints(true).size();
                uint32 apoints = dest->getPoints().size();

                out << round(dest->distanceTo(botPos));

                out << " to " << dest->getTitle();

                out << " " << apoints;
                if (apoints < tpoints)
                    out << "/" << tpoints;
                out << " points.";

                if (!dest->isActive(bot))
                    out << " not active";

                ai->TellPlayer(requester, out);

                limit++;
            }
        }
    }

    return count;
}
