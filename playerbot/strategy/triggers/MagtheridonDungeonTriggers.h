#pragma once
#include "DungeonTriggers.h"
#include "playerbot/strategy/generic/MagtheridonDungeonStrategies.h"

namespace ai
{
    class MagtheridonEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        MagtheridonEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter magtheridon's lair", "magtheridon's lair", 544) {}
    };

    class MagtheridonLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        MagtheridonLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave magtheridon's lair", "magtheridon's lair", 544) {}
    };

    class MagtheridonStartFightTrigger : public StartBossFightTrigger
    {
    public:
        MagtheridonStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start magtheridon fight", "magtheridon", MAG_NPC_MAGTHERIDON) {}
    };

    class MagtheridonEndFightTrigger : public EndBossFightTrigger
    {
    public:
        MagtheridonEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end magtheridon fight", "magtheridon", MAG_NPC_MAGTHERIDON) {}
    };

    // Blast Nova: Magtheridon casts this, must use cubes to interrupt
    class MagtheridonBlastNovaTrigger : public Trigger
    {
    public:
        MagtheridonBlastNovaTrigger(PlayerbotAI* ai) : Trigger(ai, "magtheridon blast nova", 1) {}
        bool IsActive() override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "magtheridon");
            if (!boss) return false;
            // Check if boss is casting Blast Nova
            Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            return spell && spell->m_spellInfo && spell->m_spellInfo->Id == MAG_SPELL_BLAST_NOVA;
        }
    };
}
