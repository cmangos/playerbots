#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    enum GruulsLairConstants
    {
        // Boss entries
        GL_NPC_HIGH_KING_MAULGAR   = 18831,
        GL_NPC_KROSH_FIREHAND      = 18832,
        GL_NPC_OLM_THE_SUMMONER    = 18834,
        GL_NPC_BLINDEYE_THE_SEER   = 18836,
        GL_NPC_KIGGLER_THE_CRAZED  = 18835,
        GL_NPC_GRUUL               = 19044,

        // Spells
        GL_SPELL_GROUND_SLAM       = 33525,
        GL_SPELL_GROUND_SLAM_2     = 39187,
        GL_SPELL_WHIRLWIND         = 33238,
        GL_SPELL_SHATTER           = 33654,
    };

    class GruulsLairDungeonStrategy : public Strategy
    {
    public:
        GruulsLairDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "gruul's lair"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class GruulFightStrategy : public Strategy
    {
    public:
        GruulFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "gruul"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };
}
