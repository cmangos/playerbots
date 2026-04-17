
#pragma once
#include "playerbot/strategy/generic/FollowMasterStrategy.h"

namespace ai
{   
    class WanderStrategy : public FollowMasterStrategy
    {
    public:
        WanderStrategy(PlayerbotAI* ai) : FollowMasterStrategy(ai) {}

        int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }
        std::string getName() override { return "wander"; }

#ifdef GenerateBotHelp
        std::string GetHelpName() override { return "wander"; } // Must equal internal name
        std::string GetHelpDescription() override
        {
            return "If bot is farther than 30 yd -> follow master. If within 30 yd -> move freely.";
        }
        std::vector<std::string> GetRelatedStrategies() override
        {
            return { "follow", "free", "stay", "guard" };
        }
#endif

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
