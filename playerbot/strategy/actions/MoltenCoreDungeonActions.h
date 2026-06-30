#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "UseItemAction.h"
#include "playerbot/strategy/generic/MoltenCoreDungeonStrategies.h"

namespace ai
{
    class MoltenCoreEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MoltenCoreEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable molten core strategy", "+molten core") {}
    };

    class MoltenCoreDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MoltenCoreDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable molten core strategy", "-molten core") {}
    };

    class MagmadarEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MagmadarEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable magmadar fight strategy", "+magmadar") {}
    };

    class MagmadarDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MagmadarDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable magmadar fight strategy", "-magmadar") {}
    };

    class MagmadarMoveAwayFromLavaBombAction : public MoveAwayFromHazard
    {
    public:
        MagmadarMoveAwayFromLavaBombAction(PlayerbotAI* ai) : MoveAwayFromHazard(ai, "move away from magmadar lava bomb") {}
    };

    class MagmadarMoveAwayAction : public MoveAwayFromCreature
    {
    public:
        MagmadarMoveAwayAction(PlayerbotAI* ai) : MoveAwayFromCreature(ai, "move away from magmadar", 11982, 31.0f) {}
    };

    class MoveToMCRuneAction : public MoveToAction
    {
    public:
        MoveToMCRuneAction(PlayerbotAI* ai) : MoveToAction(ai, "move to mc rune") { qualifier = "entry filter::{gos in sight,mc runes}"; }
    };

    class DouseMCRuneActionAqual : public UseItemIdAction
    {
    public:
        DouseMCRuneActionAqual(PlayerbotAI* ai) : UseItemIdAction(ai, "douse mc rune aqual") { qualifier = "{17333,entry filter::{gos close,mc runes}}"; }
    };

    class DouseMCRuneActionEternal : public UseItemIdAction
    {
    public:
        DouseMCRuneActionEternal(PlayerbotAI* ai) : UseItemIdAction(ai, "douse mc rune eternal") { qualifier = "{22754,entry filter::{gos close,mc runes}}"; }
    };

    // ----- New MC Boss Actions (ported from AC-playerbots) -----

    // Baron Geddon fight strategy enable/disable
    class BaronGeddonEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        BaronGeddonEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable baron geddon fight strategy", "+baron geddon") {}
    };

    class BaronGeddonDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        BaronGeddonDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable baron geddon fight strategy", "-baron geddon") {}
    };

    // Golemagg fight strategy enable/disable
    class GolemaggEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        GolemaggEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable golemagg fight strategy", "+golemagg") {}
    };

    class GolemaggDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        GolemaggDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable golemagg fight strategy", "-golemagg") {}
    };

    // Living Bomb: flee away from current target (effectively from group)
    class MCMoveFromGroupAction : public MovementAction
    {
    public:
        MCMoveFromGroupAction(PlayerbotAI* ai) : MovementAction(ai, "mc move from group") {}
        bool Execute(Event& event) override
        {
            // Flee away from nearest friendly player to avoid Living Bomb damage to raid
            Unit* target = AI_VALUE(Unit*, "master target");
            if (!target)
                target = ai->GetBot();
            return Flee(target);
        }
    };

    // Baron Geddon Inferno: ranged/healers move away from boss
    class MCMoveFromBaronGeddonAction : public MoveAwayFromCreature
    {
    public:
        MCMoveFromBaronGeddonAction(PlayerbotAI* ai) : MoveAwayFromCreature(ai, "mc move from baron geddon", MC_NPC_BARON_GEDDON, 25.0f) {}
    };
}