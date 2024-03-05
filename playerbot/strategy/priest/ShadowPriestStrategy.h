#pragma once

#include "HealPriestStrategy.h"

namespace ai
{
    class ShadowPriestStrategy : public GenericPriestStrategy
    {
    public:
        ShadowPriestStrategy(PlayerbotAI* ai);
        std::string getName() override { return "shadow"; }
        int GetType() override { return STRATEGY_TYPE_DPS | STRATEGY_TYPE_RANGED; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
        NextAction** GetDefaultCombatActions() override;
    };

    class ShadowPriestAoeStrategy : public CombatStrategy
    {
    public:
        ShadowPriestAoeStrategy(PlayerbotAI* ai) : CombatStrategy(ai) {}
        std::string getName() override { return "shadow aoe"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class ShadowPriestDebuffStrategy : public CombatStrategy
    {
    public:
        ShadowPriestDebuffStrategy(PlayerbotAI* ai) : CombatStrategy(ai) {}
        std::string getName() override { return "shadow debuff"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
