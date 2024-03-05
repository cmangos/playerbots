#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class FocusHealTargetStrategy : public Strategy
    {
    public:
        FocusHealTargetStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "focus heal target"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "focus heal target"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This strategy will make the bot focus heal the specified target using the 'set focus heal <targetname>' command";
        }
#endif
    };
}