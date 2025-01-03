#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "GenericActions.h"
#include "playerbot/TravelMgr.h"

namespace ai
{
    class ChooseTravelTargetAction : public MovementAction {
    public:
        ChooseTravelTargetAction(PlayerbotAI* ai, std::string name = "choose travel target") : MovementAction(ai, name) {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();

        void getNewTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget);

    protected:
        void setNewTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget);
        void ReportTravelTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget);

        std::vector<WorldPosition*> getLogicalPoints(Player* requester, std::vector<WorldPosition*>& travelPoints);
        bool SetBestTarget(Player* requester, TravelTarget* target, std::vector<TravelDestination*>& activeDestinations);

        bool SetGroupTarget(Player* requester, TravelTarget* target);
        bool SetCurrentTarget(Player* requester, TravelTarget* target, TravelTarget* oldTarget);
        bool SetQuestTarget(Player* requester, TravelTarget* target, bool newQuests = true, bool activeQuests = true, bool completedQuests = true);
        bool SetRpgTarget(Player* requester, TravelTarget* target);
        bool SetGrindTarget(Player* requester, TravelTarget* target);
        bool SetBossTarget(Player* requester, TravelTarget* target);
        bool SetExploreTarget(Player* requester, TravelTarget* target);
        bool SetNpcFlagTarget(Player* requester, TravelTarget* target, std::vector<NPCFlags> flags, std::string name = "", std::vector<uint32> items = {}, bool force = true);
        bool SetGOTypeTarget(Player* requester, TravelTarget* target, GameobjectTypes type, std::string name = "", bool force = true);
        bool SetNullTarget(TravelTarget* target);

    public:
        static TravelDestination* FindDestination(Player* bot, std::string name, bool zones = true, bool npcs = true, bool quests = true, bool mobs = true, bool bosses = true);
    private:
        virtual bool needForQuest(Unit* target);
        virtual bool needItemForQuest(uint32 itemId, const Quest* questTemplate, const QuestStatusData* questStatus);

        PlayerTravelInfo info;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "choose travel target"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action is used to select a target for the bots to travel to.\n"
                "Bots will travel to specific places for specific reasons.\n"
                "For example bots will travel to a vendor if they need to sell stuff\n"
                "The current destination or those of group members are also options.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "travel target", "group or", "should sell","can sell","can ah sell","should repair","can repair","following party","near leader","should get money","can fight equal","can fight elite","can fight boss","can free move to","rpg target","attack target",}; }
#endif 
    };

    class FocusTravelTargetAction : public ChatCommandAction {
    public:
        FocusTravelTargetAction(PlayerbotAI* ai, std::string name = "focus travel target") : ChatCommandAction(ai, name) {}

        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "focus travel target"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command specifies quests the bot should focus on.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}
