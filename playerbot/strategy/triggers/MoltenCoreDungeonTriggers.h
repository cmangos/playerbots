#pragma once
#include "DungeonTriggers.h"
#include "GenericTriggers.h"
#include "playerbot/strategy/generic/MoltenCoreDungeonStrategies.h"

namespace ai
{
    class MoltenCoreEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        MoltenCoreEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter molten core", "molten core", 409) {}
    };

    class MoltenCoreLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        MoltenCoreLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave molten core", "molten core", 409) {}
    };

    class MagmadarStartFightTrigger : public StartBossFightTrigger
    {
    public:
        MagmadarStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start magmadar fight", "magmadar", 11982) {}
    };

    class MagmadarEndFightTrigger : public EndBossFightTrigger
    {
    public:
        MagmadarEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end magmadar fight", "magmadar", 11982) {}
    };

    class MagmadarLavaBombTrigger : public CloseToGameObjectHazardTrigger
    {
    public:
        MagmadarLavaBombTrigger(PlayerbotAI* ai) : CloseToGameObjectHazardTrigger(ai, "magmadar lava bomb", 177704, 5.0f, 60) {}
    };

    class MagmadarTooCloseTrigger : public CloseToCreatureTrigger
    {
    public:
        MagmadarTooCloseTrigger(PlayerbotAI* ai) : CloseToCreatureTrigger(ai, "magmadar too close", 11982, 30.0f) {}
    };

    class FireProtectionPotionReadyTrigger : public ItemBuffReadyTrigger
    {
    public:
        FireProtectionPotionReadyTrigger(PlayerbotAI* ai) : ItemBuffReadyTrigger(ai, "fire protection potion ready", 13457, 17543) {}
    };

    class MCRuneInSightTrigger : public ValueTrigger
    {
    public:
        MCRuneInSightTrigger(PlayerbotAI* ai) : ValueTrigger(ai, "mc rune in sight", 1)
        {
            qualifier = "and::{"
                "action possible::use id::17333,"
                "has object::go usable filter::go trapped filter::entry filter::{gos in sight,mc runes},"
                "not::has object::entry filter::{gos close,mc runes}"
                "}";
        }
    };

    class MCRuneCloseTrigger : public ValueTrigger
    {
    public:
        MCRuneCloseTrigger(PlayerbotAI* ai) : ValueTrigger(ai, "mc rune close", 1) { qualifier = "has object::go usable filter::entry filter::{gos close,mc runes}"; }
    };

    // ----- New MC Boss Triggers (ported from AC-playerbots) -----

    class BaronGeddonStartFightTrigger : public StartBossFightTrigger
    {
    public:
        BaronGeddonStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start baron geddon fight", "baron geddon", MC_NPC_BARON_GEDDON) {}
    };

    class BaronGeddonEndFightTrigger : public EndBossFightTrigger
    {
    public:
        BaronGeddonEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end baron geddon fight", "baron geddon", MC_NPC_BARON_GEDDON) {}
    };

    class GolemaggStartFightTrigger : public StartBossFightTrigger
    {
    public:
        GolemaggStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start golemagg fight", "golemagg", MC_NPC_GOLEMAGG) {}
    };

    class GolemaggEndFightTrigger : public EndBossFightTrigger
    {
    public:
        GolemaggEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end golemagg fight", "golemagg", MC_NPC_GOLEMAGG) {}
    };

    // Living Bomb: Baron Geddon places on random raid member, must run away from group
    class MCLivingBombDebuffTrigger : public Trigger
    {
    public:
        MCLivingBombDebuffTrigger(PlayerbotAI* ai) : Trigger(ai, "mc living bomb debuff", 1) {}
        bool IsActive() override { return bot->HasAura(MC_SPELL_LIVING_BOMB); }
    };

    // Baron Geddon Inferno: boss channels AoE fire, ranged/healers should flee
    class MCBaronGeddonInfernoTrigger : public Trigger
    {
    public:
        MCBaronGeddonInfernoTrigger(PlayerbotAI* ai) : Trigger(ai, "mc baron geddon inferno", 1) {}
        bool IsActive() override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon");
            return boss && boss->HasAura(MC_SPELL_INFERNO);
        }
    };

    // Shazzrah: ranged DPS should stay 26+ yards away (Arcane Explosion range)
    class MCShazzrahRangedTrigger : public Trigger
    {
    public:
        MCShazzrahRangedTrigger(PlayerbotAI* ai) : Trigger(ai, "mc shazzrah ranged", 1) {}
        bool IsActive() override
        {
            return AI_VALUE2(Unit*, "find target", "shazzrah") && ai->IsRanged(bot);
        }
    };
}