#pragma once
#include "DungeonTriggers.h"
#include "playerbot/strategy/generic/ZulAmanDungeonStrategies.h"

namespace ai
{
    class ZAEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        ZAEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter zul'aman", "zul'aman", 568) {}
    };

    class ZALeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        ZALeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave zul'aman", "zul'aman", 568) {}
    };

    class AkilzonStartFightTrigger : public StartBossFightTrigger
    {
    public:
        AkilzonStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start akilzon fight", "akil'zon", ZA_NPC_AKILZON) {}
    };

    class AkilzonEndFightTrigger : public EndBossFightTrigger
    {
    public:
        AkilzonEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end akilzon fight", "akil'zon", ZA_NPC_AKILZON) {}
    };

    class JanalaiStartFightTrigger : public StartBossFightTrigger
    {
    public:
        JanalaiStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start janalai fight", "jan'alai", ZA_NPC_JANALAI) {}
    };

    class JanalaiEndFightTrigger : public EndBossFightTrigger
    {
    public:
        JanalaiEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end janalai fight", "jan'alai", ZA_NPC_JANALAI) {}
    };

    class ZuljinStartFightTrigger : public StartBossFightTrigger
    {
    public:
        ZuljinStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start zuljin fight", "zul'jin", ZA_NPC_ZULJIN) {}
    };

    class ZuljinEndFightTrigger : public EndBossFightTrigger
    {
    public:
        ZuljinEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end zuljin fight", "zul'jin", ZA_NPC_ZULJIN) {}
    };

    // Akil'zon Static Disruption: ranged should spread
    class ZAAkilzonStaticDisruptionTrigger : public Trigger
    {
    public:
        ZAAkilzonStaticDisruptionTrigger(PlayerbotAI* ai) : Trigger(ai, "za akilzon static disruption", 1) {}
        bool IsActive() override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "akil'zon");
            return boss && ai->IsRanged(bot);
        }
    };

    // Akil'zon Electrical Storm: must group under levitated player
    class ZAAkilzonElectricalStormTrigger : public Trigger
    {
    public:
        ZAAkilzonElectricalStormTrigger(PlayerbotAI* ai) : Trigger(ai, "za akilzon electrical storm", 1) {}
        bool IsActive() override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "akil'zon");
            if (!boss) return false;
            Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            return spell && spell->m_spellInfo && spell->m_spellInfo->Id == ZA_SPELL_ELECTRICAL_STORM;
        }
    };

    // Jan'alai Fire Bombs trigger
    class ZAJanalaiFireBombsTrigger : public Trigger
    {
    public:
        ZAJanalaiFireBombsTrigger(PlayerbotAI* ai) : Trigger(ai, "za janalai fire bombs", 1) {}
        bool IsActive() override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "jan'alai");
            if (!boss) return false;
            // Fire bombs phase - simplified detection
            return boss->HasAura(43648); // Jan'alai fire bomb visual
        }
    };
}
