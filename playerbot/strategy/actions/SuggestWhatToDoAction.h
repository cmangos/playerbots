#pragma once
#include "playerbot/strategy/Action.h"

namespace ai
{
    class SuggestWhatToDoAction : public Action
    {
    public:
        SuggestWhatToDoAction(PlayerbotAI* ai, string name = "suggest what to do");
        virtual bool Execute(Event& event) override;
        virtual bool isUseful();

    protected:
        typedef void (SuggestWhatToDoAction::*Suggestion) ();
        vector<Suggestion> suggestions;
        void instance();
        void specificQuest();
        void grindMaterials();
        void grindReputation();
        void something();
        void trade();
        void spam(string msg, uint8 flags = 0, bool worldChat = false, bool guild = false);

        vector<uint32> GetIncompletedQuests();

    private:
        static map<string, int> instances;
        static map<string, int> factions;
        int32 _locale;
    };

    class SuggestTradeAction : public SuggestWhatToDoAction
    {
    public:
        SuggestTradeAction(PlayerbotAI* ai) : SuggestWhatToDoAction(ai) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
    };
}
