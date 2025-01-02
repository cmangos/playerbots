
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
    PlayerTravelInfo info(bot);
    
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

        if (travelDetail != QUEST_TRAVEL_DETAIL_NONE && target->GetDestination())
        {
            if (typeid(*target->GetDestination()) == typeid(QuestRelationTravelDestination) || typeid(*target->GetDestination()) == typeid(QuestObjectiveTravelDestination))
            {
                QuestTravelDestination* QuestDestination = (QuestTravelDestination*)target->GetDestination();

                if (QuestDestination->GetQuestId() == questId)
                {
                    std::ostringstream out;

                    out << "[Active] traveling " << target->GetPosition()->distance(botPos);

                    out << " to " << QuestDestination->GetTitle();

                    ai->TellPlayer(requester, out);
                }
            }
        }       
    }

    return count;
}
