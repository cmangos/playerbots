#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class ConsumableStrategy : public Strategy
    {
    public:
        ConsumableStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "consumables"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}