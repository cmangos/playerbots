#pragma once
#include "GenericActions.h"

namespace ai
{
    class ProfessionStatusAction : public ChatCommandAction
    {
    public:
        ProfessionStatusAction(PlayerbotAI* ai) : ChatCommandAction(ai, "profession") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "profession"; }
        virtual std::string GetHelpDescription()
        {
            return "Shows persisted profession skills and the autonomous profession plan.\n"
                "Example: profession\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "profession crafting plan" }; }
#endif
    };
}
