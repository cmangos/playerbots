#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "playerbot/strategy/generic/GruulsLairDungeonStrategies.h"

namespace ai
{
    class GruulsLairEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        GruulsLairEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable gruul's lair strategy", "+gruul's lair") {}
    };

    class GruulsLairDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        GruulsLairDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable gruul's lair strategy", "-gruul's lair") {}
    };

    class GruulEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        GruulEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable gruul fight strategy", "+gruul") {}
    };

    class GruulDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        GruulDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable gruul fight strategy", "-gruul") {}
    };

    // Gruul Shatter Spread: flee away when Ground Slam debuff is active
    class GruulShatterSpreadAction : public MovementAction
    {
    public:
        GruulShatterSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "gruul shatter spread") {}
        bool Execute(Event& event) override
        {
            // Spread away from nearest player to minimize Shatter damage
            Unit* target = AI_VALUE(Unit*, "master target");
            if (!target)
                target = bot;
            return Flee(target);
        }
    };
}
