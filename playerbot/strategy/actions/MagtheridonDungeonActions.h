#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"

namespace ai
{
    class MagtheridonEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MagtheridonEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable magtheridon's lair strategy", "+magtheridon's lair") {}
    };

    class MagtheridonDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MagtheridonDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable magtheridon's lair strategy", "-magtheridon's lair") {}
    };

    class MagtheridonEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MagtheridonEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable magtheridon fight strategy", "+magtheridon") {}
    };

    class MagtheridonDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MagtheridonDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable magtheridon fight strategy", "-magtheridon") {}
    };

    // Use Manticron Cube to interrupt Blast Nova
    class MagtheridonUseCubeAction : public MovementAction
    {
    public:
        MagtheridonUseCubeAction(PlayerbotAI* ai) : MovementAction(ai, "magtheridon use cube") {}
        bool Execute(Event& event) override
        {
            // Move to nearest Manticron Cube and interact
            // This is a simplified version - full implementation would need GO interaction
            Unit* boss = AI_VALUE2(Unit*, "find target", "magtheridon");
            if (!boss) return false;
            return Flee(boss);
        }
    };
}
