#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    enum ZulAmanConstants
    {
        // Boss entries
        ZA_NPC_AKILZON      = 23574,  // Eagle
        ZA_NPC_NALORAKK     = 23576,  // Bear
        ZA_NPC_JANALAI      = 23578,  // Dragonhawk
        ZA_NPC_HALAZZI      = 23577,  // Lynx
        ZA_NPC_HEX_LORD     = 24239,  // Hex Lord Malacrass
        ZA_NPC_ZULJIN       = 23863,  // Zul'jin

        // Spells
        ZA_SPELL_STATIC_DISRUPTION = 43622,
        ZA_SPELL_ELECTRICAL_STORM  = 43648,
        ZA_SPELL_FLAME_BREATH      = 43582, // Jan'alai
        ZA_SPELL_CLAW_RAGE         = 43149, // Halazzi
    };

    class ZulAmanDungeonStrategy : public Strategy
    {
    public:
        ZulAmanDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "zul'aman"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AkilzonFightStrategy : public Strategy
    {
    public:
        AkilzonFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "akilzon"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class JanalaiFightStrategy : public Strategy
    {
    public:
        JanalaiFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "janalai"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ZuljinFightStrategy : public Strategy
    {
    public:
        ZuljinFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "zuljin"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
