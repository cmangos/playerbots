#pragma once

#include "playerbot/playerbot.h"
#include "../Trigger.h"

namespace ai
{
    // Trigger that fires when test strategy is active and test hasn't started
    class TestReadyTrigger : public Trigger
    {
    public:
        TestReadyTrigger(PlayerbotAI* ai) : Trigger(ai, "test ready") {}
        
        virtual Event Check() override
        {
            // Get test name from strategy - look for "test" prefix (could be "test" or "test:something")
            auto strategies = ai->GetStrategies(BotState::BOT_STATE_NON_COMBAT);
            for (const auto& strat : strategies)
            {
                std::string stratStr(strat);
                // Match "test" or "test:*" 
                if (stratStr == "test" || (stratStr.size() > 6 && stratStr.substr(0, 6) == "test::"))
                {
                    std::string testName = "";
                    if (stratStr.size() > 6)
                        testName = stratStr.substr(6); // Extract name after "test:"
                    
                    // Fire the trigger when test strategy is active
                    return Event("test ready", testName);
                }
            }
            return Event();
        }
    };
}
