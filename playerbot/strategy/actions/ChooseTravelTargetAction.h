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

        bool SetBestTarget(Player* requester, TravelTarget* target, PartitionedTravelList& travelPartitions, bool onlyActive = true);

        bool SetNpcFlagTarget(Player* requester, TravelTarget* target, std::vector<NPCFlags> flags, std::string name = "", std::vector<uint32> items = {}, bool force = false);
   
        bool SetNullTarget(TravelTarget* target);
    public:
        static DestinationList FindDestination(PlayerTravelInfo info, std::string name, bool zones = true, bool npcs = true, bool quests = true, bool mobs = true, bool bosses = true);
    protected:
        PlayerTravelInfo info;
        const std::vector<uint32> travelPartitions = { 100, 250, 500, 1000, 2000, 3000, 4000, 5000, 6000, 10000, 50000, 100000, 500000 };
    private:
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

    class ChooseGroupTravelTargetAction : public ChooseTravelTargetAction {
    public:
        ChooseGroupTravelTargetAction(PlayerbotAI* ai, std::string name = "choose group travel target") : ChooseTravelTargetAction(ai, name) {}

        virtual bool Execute(Event& event);
        virtual bool isUseful() { return bot->GetGroup(); }
    };

    class RefreshTravelTargetAction : public ChooseTravelTargetAction {
    public:
        RefreshTravelTargetAction(PlayerbotAI* ai, std::string name = "refresh travel target") : ChooseTravelTargetAction(ai, name) {}

        virtual bool Execute(Event& event);
        virtual bool isUseful() { return AI_VALUE(TravelTarget*, "travel target")->GetDestination()->IsActive(bot, PlayerTravelInfo(bot)) && (!WorldPosition(bot).isOverworld() || urand(1, 100) > 10); }
    };

    using FutureDestinations = std::future<PartitionedTravelList>;

    class ChooseAsyncTravelTargetAction : public ChooseTravelTargetAction, public Qualified {
    public:
        ChooseAsyncTravelTargetAction(PlayerbotAI* ai, std::string name = "choose async travel target") : ChooseTravelTargetAction(ai, name), Qualified() {}
    protected:
        TravelDestinationPurpose actionPurpose = TravelDestinationPurpose::None;

        virtual bool RequestNewDestinations(Event& event);
        virtual bool PassTrough() const { return false; } //We need this skip on the request instead of isUsefull to only skip the request sometimes but not the Wait and Set that could follow a nonskip.

        bool hasDestinations = false;
        PartitionedTravelList destinationList = {};
        FutureDestinations futureDestinations;
    private:
        virtual bool Execute(Event& event) override;

        virtual bool isUseful() override;
        bool WaitForDestinations();
        bool SetBestDestination(Event& event);
    };

    class ChooseAsyncNamedTravelTargetAction : public ChooseAsyncTravelTargetAction {
    public:
        ChooseAsyncNamedTravelTargetAction(PlayerbotAI* ai, std::string name = "choose async named travel target") : ChooseAsyncTravelTargetAction(ai, name) {}
    protected:
        virtual bool PassTrough() const override;
        virtual bool RequestNewDestinations(Event& event) override;
        virtual bool isUseful() override { return ChooseTravelTargetAction::isUseful(); };
    };

    class ChooseAsyncQuestTravelTargetAction : public ChooseAsyncTravelTargetAction {
    public:
        ChooseAsyncQuestTravelTargetAction(PlayerbotAI* ai, std::string name = "choose async quest travel target") : ChooseAsyncTravelTargetAction(ai, name) {}
    protected:
        virtual bool RequestNewDestinations(Event& event) override;
        virtual bool isUseful() override { return ChooseTravelTargetAction::isUseful(); };
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
