#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "playerbot/strategy/generic/ZulAmanDungeonStrategies.h"

namespace ai
{
    class ZAEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        ZAEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable zul'aman strategy", "+zul'aman") {}
    };

    class ZADisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        ZADisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable zul'aman strategy", "-zul'aman") {}
    };

    class AkilzonEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        AkilzonEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable akilzon fight strategy", "+akilzon") {}
    };

    class AkilzonDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        AkilzonDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable akilzon fight strategy", "-akilzon") {}
    };

    class JanalaiEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        JanalaiEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable janalai fight strategy", "+janalai") {}
    };

    class JanalaiDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        JanalaiDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable janalai fight strategy", "-janalai") {}
    };

    class ZuljinEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        ZuljinEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable zuljin fight strategy", "+zuljin") {}
    };

    class ZuljinDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        ZuljinDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable zuljin fight strategy", "-zuljin") {}
    };

    // Akil'zon: spread for Static Disruption
    class ZAAkilzonSpreadAction : public MovementAction
    {
    public:
        ZAAkilzonSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "za akilzon spread") {}
        bool Execute(Event& event) override
        {
            Unit* target = AI_VALUE(Unit*, "master target");
            if (!target) target = bot;
            return Flee(target);
        }
    };

    // Akil'zon: move under Electrical Storm target
    class ZAAkilzonMoveToStormAction : public MovementAction
    {
    public:
        ZAAkilzonMoveToStormAction(PlayerbotAI* ai) : MovementAction(ai, "za akilzon move to storm") {}
        bool Execute(Event& event) override
        {
            // Move toward the boss (storm is near center)
            Unit* boss = AI_VALUE2(Unit*, "find target", "akil'zon");
            if (!boss) return false;
            return ChaseTo(boss, 5.0f);
        }
    };

    // Jan'alai: avoid fire bombs
    class ZAJanalaiAvoidFireBombsAction : public MovementAction
    {
    public:
        ZAJanalaiAvoidFireBombsAction(PlayerbotAI* ai) : MovementAction(ai, "za janalai avoid fire bombs") {}
        bool Execute(Event& event) override
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "jan'alai");
            if (!boss) return false;
            return Flee(boss);
        }
    };
}
