#pragma once

#include "playerbot/strategy/Action.h"
#include "Quests/QuestDef.h"

namespace ai
{
    class QuestAction : public Action 
    {
    public:
        QuestAction(PlayerbotAI* ai, string name) : Action(ai, name) {}

    public:
        virtual bool Execute(Event& event);

    protected:
        virtual bool ProcessQuest(Player* requester, Quest const* quest, WorldObject* questGiver) = 0;
        bool CompleteQuest(Player* player, uint32 entry);
        bool AcceptQuest(Player* requester, Quest const* quest, uint64 questGiver);
        bool ProcessQuests(ObjectGuid questGiver);
        bool ProcessQuests(WorldObject* questGiver);
    };
    
    class QuestObjectiveCompletedAction : public Action 
    {
    public:
        QuestObjectiveCompletedAction(PlayerbotAI* ai) : Action(ai, "quest objective completed") {}

    public:
        virtual bool Execute(Event& event);
    };
}
