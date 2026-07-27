#pragma once
#include "playerbot/LootObjectStack.h"
#include "GenericActions.h"

namespace ai
{
    class AvoidCreatureListAction : public ChatCommandAction
    {
    public:
        AvoidCreatureListAction(PlayerbotAI* ai) : ChatCommandAction(ai, "avoid creature list") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "avoid creature list"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This chat command gives control over the list of creatures a bot should stay away from.\n"
                "Examples:\n"
                "avoid creature ? : List the creatures to avoid.\n"
                "avoid creature [creature id] : Stay away from this creature.\n"
                "avoid creature -[creature id] : Remove this creature id from the list.\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "avoid creature list" }; }
#endif 

    private:
        std::vector<std::string> ParseCreatures(const std::string& text);
    };
}
