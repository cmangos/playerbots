#pragma once
#include "DungeonTriggers.h"
#include "playerbot/strategy/generic/TempestKeepDungeonStrategies.h"

namespace ai
{
    class TKEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        TKEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter tempest keep", "tempest keep", 550) {}
    };

    class TKLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        TKLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave tempest keep", "tempest keep", 550) {}
    };

    class VoidReaverStartFightTrigger : public StartBossFightTrigger
    {
    public:
        VoidReaverStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start void reaver fight", "void reaver", TK_NPC_VOID_REAVER) {}
    };

    class VoidReaverEndFightTrigger : public EndBossFightTrigger
    {
    public:
        VoidReaverEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end void reaver fight", "void reaver", TK_NPC_VOID_REAVER) {}
    };

    class SolarianStartFightTrigger : public StartBossFightTrigger
    {
    public:
        SolarianStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start solarian fight", "high astromancer solarian", TK_NPC_SOLARIAN) {}
    };

    class SolarianEndFightTrigger : public EndBossFightTrigger
    {
    public:
        SolarianEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end solarian fight", "high astromancer solarian", TK_NPC_SOLARIAN) {}
    };

    class KaelthasStartFightTrigger : public StartBossFightTrigger
    {
    public:
        KaelthasStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start kaelthas fight", "kael'thas sunstrider", TK_NPC_KAELTHAS) {}
    };

    class KaelthasEndFightTrigger : public EndBossFightTrigger
    {
    public:
        KaelthasEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end kaelthas fight", "kael'thas sunstrider", TK_NPC_KAELTHAS) {}
    };

    // Void Reaver Arcane Orb: targeted on random player
    class TKVoidReaverArcaneOrbTrigger : public Trigger
    {
    public:
        TKVoidReaverArcaneOrbTrigger(PlayerbotAI* ai) : Trigger(ai, "tk void reaver arcane orb", 1) {}
        bool IsActive() override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
            if (!boss) return false;
            // Detect Arcane Orb cast targeting this player
            Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (!spell || !spell->m_spellInfo || spell->m_spellInfo->Id != TK_SPELL_ARCANE_ORB)
                return false;
            Unit* target = spell->m_targets.getUnitTarget();
            return target && target->GetObjectGuid() == bot->GetObjectGuid();
        }
    };

    // Solarian Wrath of the Astromancer debuff: bot has bomb
    class TKSolarianWrathDebuffTrigger : public Trigger
    {
    public:
        TKSolarianWrathDebuffTrigger(PlayerbotAI* ai) : Trigger(ai, "tk solarian wrath debuff", 1) {}
        bool IsActive() override
        {
            return bot->HasAura(TK_SPELL_WRATH_ASTROMANCER);
        }
    };
}
