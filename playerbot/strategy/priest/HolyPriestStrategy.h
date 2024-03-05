#pragma once

#include "HealPriestStrategy.h"

namespace ai
{
    class HolyPriestStrategy : public HealPriestStrategy
    {
    public:
        HolyPriestStrategy(PlayerbotAI* ai);
        std::string getName() override { return "holy"; }
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
        NextAction** GetDefaultCombatActions() override;
    };
}
