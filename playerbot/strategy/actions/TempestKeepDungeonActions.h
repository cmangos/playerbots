#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "playerbot/strategy/generic/TempestKeepDungeonStrategies.h"

namespace ai
{
    class TKEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        TKEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable tempest keep strategy", "+tempest keep") {}
    };

    class TKDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        TKDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable tempest keep strategy", "-tempest keep") {}
    };

    class VoidReaverEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        VoidReaverEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable void reaver fight strategy", "+void reaver") {}
    };

    class VoidReaverDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        VoidReaverDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable void reaver fight strategy", "-void reaver") {}
    };

    class SolarianEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        SolarianEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable solarian fight strategy", "+solarian") {}
    };

    class SolarianDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        SolarianDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable solarian fight strategy", "-solarian") {}
    };

    class KaelthasEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        KaelthasEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable kaelthas fight strategy", "+kaelthas") {}
    };

    class KaelthasDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        KaelthasDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable kaelthas fight strategy", "-kaelthas") {}
    };

    // Void Reaver: flee from Arcane Orb
    class TKVoidReaverAvoidOrbAction : public MovementAction
    {
    public:
        TKVoidReaverAvoidOrbAction(PlayerbotAI* ai) : MovementAction(ai, "tk void reaver avoid orb") {}
        bool Execute(Event& event) override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "void reaver");
            if (!boss) return false;
            return Flee(boss);
        }
    };

    // Solarian: move away from group when having Wrath bomb
    class TKSolarianMoveFromGroupAction : public MovementAction
    {
    public:
        TKSolarianMoveFromGroupAction(PlayerbotAI* ai) : MovementAction(ai, "tk solarian move from group") {}
        bool Execute(Event& event) override
        {
            Unit* target = AI_VALUE(Unit*, "master target");
            if (!target)
                target = bot;
            return Flee(target);
        }
    };
}
