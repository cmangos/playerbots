#pragma once
#include "playerbot/strategy/Strategy.h"
#include "playerbot/strategy/Value.h"
#include "playerbot/strategy/Multiplier.h"

namespace ai
{
    // General Ahn'Qiraj (AQ20) Dungeon Strategy
    class AhnQirajDungeonStrategy : public Strategy
    {
    public:
        AhnQirajDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "ahn'qiraj"; }
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    // Kurinnaxx Fight Specific Strategy
    class KurinnaxxFightStrategy : public Strategy
    {
    public:
        KurinnaxxFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "kurinnaxx"; }
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}