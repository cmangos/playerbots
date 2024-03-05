#pragma once
#include "playerbot/strategy/Strategy.h"
#include "playerbot/strategy/generic/CombatStrategy.h"

namespace ai
{
    class GenericPriestStrategy : public CombatStrategy
    {
    public:
        GenericPriestStrategy(PlayerbotAI* ai);

    protected:
        virtual void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class PriestCureStrategy : public Strategy
    {
    public:
        PriestCureStrategy(PlayerbotAI* ai);
        std::string getName() override { return "cure"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class PriestBoostStrategy : public Strategy
    {
    public:
        PriestBoostStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "boost"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class PriestCcStrategy : public Strategy
    {
    public:
        PriestCcStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "cc"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
