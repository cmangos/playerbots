#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    enum MagtheridonConstants
    {
        MAG_NPC_MAGTHERIDON         = 17257,
        MAG_NPC_HELLFIRE_CHANNELER  = 17256,
        MAG_NPC_BURNING_ABYSSAL     = 17454,
        MAG_NPC_MANTICRON_CUBE      = 181713, // GO entry

        MAG_SPELL_BLAST_NOVA        = 30616,
        MAG_SPELL_QUAKE             = 30657,
        MAG_SPELL_CLEAVE            = 30619,
    };

    class MagtheridonDungeonStrategy : public Strategy
    {
    public:
        MagtheridonDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "magtheridon's lair"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class MagtheridonFightStrategy : public Strategy
    {
    public:
        MagtheridonFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "magtheridon"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
