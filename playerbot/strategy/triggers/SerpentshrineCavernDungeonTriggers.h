#pragma once
#include "DungeonTriggers.h"
#include "playerbot/strategy/generic/SerpentshrineCavernDungeonStrategies.h"

namespace ai
{
    class SSCEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        SSCEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter serpentshrine cavern", "serpentshrine cavern", 548) {}
    };

    class SSCLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        SSCLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave serpentshrine cavern", "serpentshrine cavern", 548) {}
    };

    class LurkerBelowStartFightTrigger : public StartBossFightTrigger
    {
    public:
        LurkerBelowStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start lurker below fight", "the lurker below", SSC_NPC_LURKER_BELOW) {}
    };

    class LurkerBelowEndFightTrigger : public EndBossFightTrigger
    {
    public:
        LurkerBelowEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end lurker below fight", "the lurker below", SSC_NPC_LURKER_BELOW) {}
    };

    class LeotherasStartFightTrigger : public StartBossFightTrigger
    {
    public:
        LeotherasStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start leotheras fight", "leotheras the blind", SSC_NPC_LEOTHERAS) {}
    };

    class LeotherasEndFightTrigger : public EndBossFightTrigger
    {
    public:
        LeotherasEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end leotheras fight", "leotheras the blind", SSC_NPC_LEOTHERAS) {}
    };

    class VashjStartFightTrigger : public StartBossFightTrigger
    {
    public:
        VashjStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start vashj fight", "lady vashj", SSC_NPC_VASHJ) {}
    };

    class VashjEndFightTrigger : public EndBossFightTrigger
    {
    public:
        VashjEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end vashj fight", "lady vashj", SSC_NPC_VASHJ) {}
    };

    // Lurker Spout: boss rotates and damages anyone in line of sight
    class SSCLurkerSpoutTrigger : public Trigger
    {
    public:
        SSCLurkerSpoutTrigger(PlayerbotAI* ai) : Trigger(ai, "ssc lurker spout", 1) {}
        bool IsActive() override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "the lurker below");
            if (!boss) return false;
            Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            return spell && spell->m_spellInfo && spell->m_spellInfo->Id == SSC_SPELL_SPOUT;
        }
    };

    // Leotheras Whirlwind: boss spins, melee must flee
    class SSCLeotherasWhirlwindTrigger : public Trigger
    {
    public:
        SSCLeotherasWhirlwindTrigger(PlayerbotAI* ai) : Trigger(ai, "ssc leotheras whirlwind", 1) {}
        bool IsActive() override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "leotheras the blind");
            if (!boss) return false;
            return boss->HasAura(SSC_SPELL_WHIRLWIND) && !ai->IsRanged(bot);
        }
    };

    // Vashj Static Charge: debuff on bot
    class SSCVashjStaticChargeTrigger : public Trigger
    {
    public:
        SSCVashjStaticChargeTrigger(PlayerbotAI* ai) : Trigger(ai, "ssc vashj static charge", 1) {}
        bool IsActive() override
        {
            return bot->HasAura(SSC_SPELL_STATIC_CHARGE);
        }
    };
}
