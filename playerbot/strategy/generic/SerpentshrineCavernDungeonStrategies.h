#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    enum SSCConstants
    {
        // Boss entries
        SSC_NPC_HYDROSS        = 21216,
        SSC_NPC_LURKER_BELOW   = 21217,
        SSC_NPC_LEOTHERAS      = 21215,
        SSC_NPC_FATHOM_LORD    = 21214,
        SSC_NPC_MOROGRIM       = 21213,
        SSC_NPC_VASHJ          = 21212,

        // Spells
        SSC_SPELL_SPOUT        = 37433,
        SSC_SPELL_WHIRLWIND    = 37640, // Leotheras
        SSC_SPELL_STATIC_CHARGE = 38280, // Vashj
    };

    class SerpentshrineCavernDungeonStrategy : public Strategy
    {
    public:
        SerpentshrineCavernDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "serpentshrine cavern"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class LurkerBelowFightStrategy : public Strategy
    {
    public:
        LurkerBelowFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "lurker below"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class LeotherasFightStrategy : public Strategy
    {
    public:
        LeotherasFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "leotheras"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class VashjFightStrategy : public Strategy
    {
    public:
        VashjFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "vashj"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
