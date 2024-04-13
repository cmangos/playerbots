#pragma once
#include "../Strategy.h"

namespace ai
{
    class AQRuinsDungeonStrategy : public Strategy
    {
    public:
        AQRuinsDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "aq ruins"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class KurinnaxxFightStrategy : public Strategy
    {
    public:
       KurinnaxxFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
       std::string getName() override { return "kurinnaxx"; }

    private:
       void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
       void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
       void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
       void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
       void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };

    class OssirianFightStrategy : public Strategy
    {
    public:
        OssirianFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "ossirian"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}