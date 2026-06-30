#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    enum TempestKeepConstants
    {
        // Boss entries
        TK_NPC_ALAR             = 19514,
        TK_NPC_VOID_REAVER      = 19516,
        TK_NPC_SOLARIAN         = 18805,
        TK_NPC_KAELTHAS         = 19622,

        // Spells
        TK_SPELL_FLAME_QUILLS   = 34229,
        TK_SPELL_ARCANE_ORB     = 29990, // Void Reaver
        TK_SPELL_WRATH_ASTROMANCER = 33045, // Solarian debuff
        TK_SPELL_POUNDING       = 34162,
    };

    class TempestKeepDungeonStrategy : public Strategy
    {
    public:
        TempestKeepDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "tempest keep"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class VoidReaverFightStrategy : public Strategy
    {
    public:
        VoidReaverFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "void reaver"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class SolarianFightStrategy : public Strategy
    {
    public:
        SolarianFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "solarian"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class KaelthasFightStrategy : public Strategy
    {
    public:
        KaelthasFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "kaelthas"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
