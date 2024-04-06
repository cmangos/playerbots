#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class FocusHealTargetsStrategy : public Strategy
    {
    public:
        FocusHealTargetsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "focus heal targets"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "focus heal targets"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This strategy will make the bot focus heal the specified target using the 'set focus heal <targetname>' command";
        }
#endif
    };
}