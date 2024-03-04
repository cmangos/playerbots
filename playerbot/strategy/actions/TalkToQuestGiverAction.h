#pragma once

#include "playerbot/strategy/Action.h"
#include "QuestAction.h"

namespace ai
{
    class TalkToQuestGiverAction : public QuestAction 
    {
    public:
        TalkToQuestGiverAction(PlayerbotAI* ai) : QuestAction(ai, "talk to quest giver") {}

    protected:
        virtual bool ProcessQuest(Player* requester, Quest const* quest, WorldObject* questGiver) override;

    private:        
        bool TurnInQuest(Player* requester, Quest const* quest, WorldObject* questGiver, string& out);
        void RewardNoItem(Quest const* quest, WorldObject* questGiver, string& out);
        void RewardSingleItem(Quest const* quest, WorldObject* questGiver, string& out);
        set<uint32> BestRewards(Quest const* quest);
        void RewardMultipleItem(Player* requester, Quest const* quest, WorldObject* questGiver, string& out);
        void AskToSelectReward(Player* requester, Quest const* quest, string& out, bool forEquip);
    };
}