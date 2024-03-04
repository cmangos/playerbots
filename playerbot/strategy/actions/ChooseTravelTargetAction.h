#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"

namespace ai
{
    class TravelTarget;
    class TravelDestination;

    class ChooseTravelTargetAction : public MovementAction {
    public:
        ChooseTravelTargetAction(PlayerbotAI* ai, string name = "choose travel target") : MovementAction(ai, name) {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();

        void getNewTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget);

    protected:
        void setNewTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget);
        void ReportTravelTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget);

        vector<WorldPosition*> getLogicalPoints(Player* requester, vector<WorldPosition*>& travelPoints);
        bool SetBestTarget(Player* requester, TravelTarget* target, vector<TravelDestination*>& activeDestinations);

        bool SetGroupTarget(Player* requester, TravelTarget* target);
        bool SetCurrentTarget(Player* requester, TravelTarget* target, TravelTarget* oldTarget);
        bool SetQuestTarget(Player* requester, TravelTarget* target, bool newQuests = true, bool activeQuests = true, bool completedQuests = true);
        bool SetRpgTarget(Player* requester, TravelTarget* target);
        bool SetGrindTarget(Player* requester, TravelTarget* target);
        bool SetBossTarget(Player* requester, TravelTarget* target);
        bool SetExploreTarget(Player* requester, TravelTarget* target);
        bool SetNpcFlagTarget(Player* requester, TravelTarget* target, vector<NPCFlags> flags, string name = "", vector<uint32> items = {}, bool force = true);
        bool SetGOTypeTarget(Player* requester, TravelTarget* target, GameobjectTypes type, string name = "", bool force = true);
        bool SetNullTarget(TravelTarget* target);

    public:
        static TravelDestination* FindDestination(Player* bot, string name, bool zones = true, bool npcs = true, bool quests = true, bool mobs = true, bool bosses = true);
    private:
        virtual bool needForQuest(Unit* target);
        virtual bool needItemForQuest(uint32 itemId, const Quest* questTemplate, const QuestStatusData* questStatus);

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "move to rpg target"; } //Must equal iternal name
        virtual string GetHelpDescription()
        {
            return "This action is used to select a target for the bots to travel to.\n"
                "Bots will travel to specific places for specific reasons.\n"
                "For example bots will travel to a vendor if they need to sell stuff\n"
                "The current destination or those of group members are also options.";
        }
        virtual vector<string> GetUsedActions() { return {}; }
        virtual vector<string> GetUsedValues() { return { "travel target", "group or", "should sell","can sell","can ah sell","should repair","can repair","following party","near leader","should get money","can fight equal","can fight elite","can fight boss","can free move to","rpg target","attack target",}; }
#endif 
    };
}
