#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Molten Core spell/NPC constants
    enum MoltenCoreConstants
    {
        // Boss entries
        MC_NPC_LUCIFRON         = 12118,
        MC_NPC_MAGMADAR         = 11982,
        MC_NPC_GEHENNAS         = 12259,
        MC_NPC_GARR             = 12057,
        MC_NPC_BARON_GEDDON     = 12056,
        MC_NPC_SHAZZRAH         = 12264,
        MC_NPC_SULFURON         = 12098,
        MC_NPC_GOLEMAGG         = 11988,
        MC_NPC_MAJORDOMO        = 12018,
        MC_NPC_RAGNAROS         = 11502,
        MC_NPC_CORE_RAGER       = 11672,

        // Spell IDs
        MC_SPELL_LIVING_BOMB    = 20475,
        MC_SPELL_INFERNO        = 19695,
        MC_SPELL_GOLEMAGGS_TRUST = 20553,
    };

    class MoltenCoreDungeonStrategy : public Strategy
    {
    public:
        MoltenCoreDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "molten core"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };

    class MagmadarFightStrategy : public Strategy
    {
    public:
        MagmadarFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "magmadar"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };

    class BaronGeddonFightStrategy : public Strategy
    {
    public:
        BaronGeddonFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "baron geddon"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };

    class GolemaggFightStrategy : public Strategy
    {
    public:
        GolemaggFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "golemagg"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}