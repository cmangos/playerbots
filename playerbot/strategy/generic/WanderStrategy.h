
#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class TriggerNode;

    class WanderStrategy : public Strategy
    {
    public:
        WanderStrategy(PlayerbotAI* ai) : Strategy(ai) {}

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
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;

        void OnStrategyAdded(BotState state) override;
        void OnStrategyRemoved(BotState state) override;
    };
}
