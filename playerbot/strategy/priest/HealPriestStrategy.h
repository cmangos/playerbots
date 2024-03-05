#pragma once

#include "GenericPriestStrategy.h"

namespace ai
{
    class HealPriestStrategy : public GenericPriestStrategy
    {
    public:
        HealPriestStrategy(PlayerbotAI* ai);
        virtual std::string getName() override { return "heal"; }
		virtual int GetType() override { return STRATEGY_TYPE_HEAL; }

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
